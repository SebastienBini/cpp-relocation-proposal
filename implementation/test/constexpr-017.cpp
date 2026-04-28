/// constexpr relocate-only type: move and copy deleted, only reloc ctor.
/// Verifies relocate-only types work in constexpr context.

#include <iostream>
#include "cxlog.h"

struct RelocOnly {
    int *p;
    constexpr RelocOnly(int *q) : p(q) {}
    RelocOnly(const RelocOnly&) = delete;
    RelocOnly(RelocOnly&&) = delete;
    constexpr RelocOnly(RelocOnly reloc src) : p(src.p) {}
    constexpr ~RelocOnly() { if (p) *p += 1; }
};

constexpr int test() {
    int counter = 0;
    {
        RelocOnly a(&counter);
        RelocOnly b = reloc a;    // reloc ctor; a cleanup deactivated
    }                              // ~b fires (counter=1); no ~a
    return counter;
}

static_assert(test() == 1, "");

// cxlog trace: mandatory elision → no reloc ctor, single dtor.
struct RL {
    cxlog* log;
    constexpr RL(cxlog& l) : log(&l)
        { log->record('R', cxlog::CTOR, this); }
    RL(const RL&) = delete;
    RL(RL&&) = delete;
    constexpr RL(RL reloc src) : log(src.log)
        { log->record('R', cxlog::RELOC, this, &src); }
    constexpr ~RL() { log->record('R', cxlog::DTOR, this); }
};

constexpr int test_log() {
    cxlog log;
    { RL a(log); RL b = reloc a; (void)b; }
    return log.match_ops("R(); ~R()");
}
static_assert(test_log() == 1, "mandatory elision: reloc-only, no ctor called");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    // cxlog trace
    cxlog log;
    { RL a(log); RL b = reloc a; (void)b; }
    std::cout << "trace: "; log.dump(); std::cout << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 1
// runtime: 1
// trace: R() 0x1; ~R() 0x1
