/// constexpr copy-only fallback: move deleted, reloc defaulted (trivial).
/// Copy adds +100, defaulted reloc is trivial bitwise copy → reloc wins.

#include <iostream>
#include "cxlog.h"

struct S {
    int val;
    constexpr S(int v) : val(v) {}
    constexpr S(const S& o) : val(o.val + 100) {}
    S(S&&) = delete;
    constexpr S(S reloc) = default;
};

constexpr int test() {
    S s(5);
    S t = reloc s;
    return t.val;
}

// reloc ctor is defaulted (trivial bitwise copy) → val = 5
static_assert(test() == 5, "");

// cxlog trace: mandatory elision → no copy, no reloc ctor.
struct SL {
    cxlog* log; int val;
    constexpr SL(cxlog& l, int v) : log(&l), val(v)
        { log->record('S', cxlog::CTOR, this); }
    constexpr SL(const SL& o) : log(o.log), val(o.val + 100)
        { log->record('S', cxlog::COPY, this, &o); }
    SL(SL&&) = delete;
    constexpr SL(SL reloc src) : log(src.log), val(src.val)
        { log->record('S', cxlog::RELOC, this, &src); }
    constexpr ~SL() { log->record('S', cxlog::DTOR, this); }
};

constexpr int test_log() {
    cxlog log;
    { SL s(log, 5); SL t = reloc s; (void)t; }
    return log.match_ops("S(); ~S()");
}
static_assert(test_log() == 1, "mandatory elision: no copy, no reloc");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    // cxlog trace
    cxlog log;
    { SL s(log, 5); SL t = reloc s; (void)t; }
    std::cout << "trace: "; log.dump(); std::cout << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 5
// runtime: 5
// trace: S() 0x1; ~S() 0x1
