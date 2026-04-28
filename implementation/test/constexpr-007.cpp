/// constexpr move-ctor fallback — source remains alive, dtor fires for both.
/// Counter should be 1 (only the live pointer increments).

#include <iostream>
#include "cxlog.h"

struct S {
    int *p;
    constexpr S(int *q) : p(q) {}
    constexpr S(S&& o) : p(o.p) { o.p = nullptr; }
    constexpr ~S() { if (p) *p += 1; }
};

constexpr int test() {
    int counter = 0;
    {
        S s(&counter);
        S t = reloc s;  // move: t.p = &counter, s.p = null
    }                    // ~s no-op (null p); ~t fires (counter=1)
    return counter;
}

static_assert(test() == 1, "");

// cxlog trace: mandatory elision → no move ctor, single dtor.
struct SL {
    cxlog* log;
    constexpr SL(cxlog& l) : log(&l)
        { log->record('S', cxlog::CTOR, this); }
    constexpr SL(SL&& o) : log(o.log)
        { log->record('S', cxlog::MOVE, this, &o); }
    constexpr ~SL() { log->record('S', cxlog::DTOR, this); }
};

constexpr int test_log() {
    cxlog log;
    { SL s(log); SL t = reloc s; (void)t; }
    return log.match_ops("S(); ~S()");
}
static_assert(test_log() == 1, "mandatory elision: move ctor not called");

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
