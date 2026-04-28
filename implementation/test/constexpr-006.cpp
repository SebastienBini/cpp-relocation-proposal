/// constexpr reloc ctor consumes source — no double dtor.
/// Counter incremented by dtor; should fire exactly once (for the target).

#include <iostream>
#include "cxlog.h"

struct S {
    int *p;
    constexpr S(int *q) : p(q) {}
    constexpr S(S reloc src) : p(src.p) { src.p = nullptr; }
    constexpr ~S() { if (p) *p += 1; }
};

constexpr int test() {
    int counter = 0;
    {
        S s(&counter);
        S t = reloc s;  // reloc ctor: t gets p, s.p nulled; s cleanup deactivated
    }                    // ~t fires (counter=1); s dtor suppressed
    return counter;
}

static_assert(test() == 1, "");

// cxlog trace: mandatory elision → no reloc ctor, single dtor.
struct SL {
    cxlog* log;
    constexpr SL(cxlog& l) : log(&l)
        { log->record('S', cxlog::CTOR, this); }
    constexpr SL(SL reloc src) : log(src.log)
        { log->record('S', cxlog::RELOC, this, &src); }
    constexpr ~SL() { log->record('S', cxlog::DTOR, this); }
};

constexpr int test_log() {
    cxlog log;
    { SL s(log); SL t = reloc s; (void)t; }
    return log.match_ops("S(); ~S()");
}
static_assert(test_log() == 1, "mandatory elision: no double dtor");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    // cxlog trace
    cxlog log;
    { SL s(log); SL t = reloc s; (void)t; }
    std::cout << "trace: "; log.dump(); std::cout << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 1
// runtime: 1
// trace: S() 0x1; ~S() 0x1
