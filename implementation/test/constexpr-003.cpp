/// constexpr reloc with user-provided reloc ctor that modifies the value.
/// Mandatory relocation elision (§reloc-elision-mandatory): local-to-local
/// reloc elides the ctor both at compile time and at runtime.

#include <iostream>
#include "cxlog.h"

struct S {
    int val;
    constexpr S(int v) : val(v) {}
    constexpr S(S reloc src) : val(src.val + 1) {}
    constexpr ~S() {}
};

constexpr int test() {
    S s(10);
    S t = reloc s;
    return t.val;
}

// Mandatory elision: reloc ctor is not called.
static_assert(test() == 10, "");

// cxlog trace: only ctor + dtor, no reloc ctor.
struct SL {
    cxlog* log; int val;
    constexpr SL(cxlog& l, int v) : log(&l), val(v)
        { log->record('S', cxlog::CTOR, this); }
    constexpr SL(SL reloc src) : log(src.log), val(src.val + 1)
        { log->record('S', cxlog::RELOC, this, &src); }
    constexpr ~SL() { log->record('S', cxlog::DTOR, this); }
};

constexpr int test_log() {
    cxlog log;
    { SL s(log, 10); SL t = reloc s; (void)t; }
    return log.match_ops("S(); ~S()");
}
static_assert(test_log() == 1, "reloc ctor must be elided");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    // cxlog trace
    cxlog log;
    { SL s(log, 10); SL t = reloc s; (void)t; }
    std::cout << "trace: "; log.dump(); std::cout << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 10
// runtime: 10
// trace: S() 0x1; ~S() 0x1
