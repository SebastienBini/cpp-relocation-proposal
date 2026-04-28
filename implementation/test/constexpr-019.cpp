/// constexpr nested scope reloc: inner scope relocs, outer scope uses result.
/// Mandatory relocation elision (§reloc-elision-mandatory): local-to-local
/// reloc elides the ctor both at compile time and at runtime.

#include <iostream>
#include "cxlog.h"

struct S {
    int v;
    constexpr S(int x) : v(x) {}
    constexpr S(S reloc src) : v(src.v + 1) {}
    constexpr S& operator=(S&& rhs) { v = rhs.v; return *this; }
    constexpr ~S() {}
};

constexpr int test() {
    S a(10);
    S b(0);
    {
        S c = reloc a;   // mandatory elision: c.v = 10
        b = reloc c;     // mandatory elision into temp: temp.v = 10; move-assign to b
    }
    return b.v;
}

// Mandatory elision: reloc ctor is not called.
static_assert(test() == 10, "");

// cxlog trace: two ctors (a, b), reloc a→c elided, reloc c→temp elided,
// move-assign fires, dtor of temp (c), dtor of b, dtor of a (already relocated).
struct SL {
    cxlog* log; int v;
    constexpr SL(cxlog& l, int x) : log(&l), v(x)
        { log->record('S', cxlog::CTOR, this); }
    constexpr SL(SL reloc src) : log(src.log), v(src.v + 1)
        { log->record('S', cxlog::RELOC, this, &src); }
    constexpr SL& operator=(SL&& rhs) { v = rhs.v; return *this; }
    constexpr ~SL() { log->record('S', cxlog::DTOR, this); }
};

constexpr int test_log() {
    cxlog log;
    {
        SL a(log, 10);
        SL b(log, 0);
        {
            SL c = reloc a;
            b = reloc c;
        }
        (void)b;
    }
    // Two ctors (a, b). Inner scope: c=reloc a elided, temp=reloc c elided,
    // temp dtor fires. Then b dtor fires.
    return log.match_ops("S(); S(); ~S(); ~S()");
}
static_assert(test_log() == 1, "nested scope: elided relocs");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    // cxlog trace
    {
        cxlog log;
        {
            SL a(log, 10);
            SL b(log, 0);
            {
                SL c = reloc a;
                b = reloc c;
            }
            (void)b;
        }
        std::cout << "trace: "; log.dump(); std::cout << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// compile_time: 10
// runtime: 10
// trace: S() 0x1; S() 0x2; ~S() 0x1; ~S() 0x2
