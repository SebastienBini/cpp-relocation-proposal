/// constexpr move-ctor fallback (no reloc ctor, trivial move).
/// Also tests non-trivial move-ctor fallback (user-provided move adds +5).
/// Mandatory relocation elision (§reloc-elision-mandatory): local-to-local
/// reloc elides any ctor (including move) at both compile time and runtime.

#include <iostream>
#include "cxlog.h"

struct Trivial {
    int val;
    constexpr Trivial(int v) : val(v) {}
    // no reloc ctor — trivial move used
};

struct NonTrivial {
    int val;
    constexpr NonTrivial(int v) : val(v) {}
    constexpr NonTrivial(NonTrivial&& o) : val(o.val + 5) {}
    constexpr ~NonTrivial() {}
};

constexpr int test_trivial() {
    Trivial t(7);
    Trivial u = reloc t;
    return u.val;
}

constexpr int test_nontrivial() {
    NonTrivial n(10);
    NonTrivial m = reloc n;
    return m.val;
}

static_assert(test_trivial() == 7, "");
// Mandatory elision: move ctor not called.
static_assert(test_nontrivial() == 10, "");

// cxlog trace for NonTrivial: move ctor is elided too.
struct NTL {
    cxlog* log; int val;
    constexpr NTL(cxlog& l, int v) : log(&l), val(v)
        { log->record('N', cxlog::CTOR, this); }
    constexpr NTL(NTL&& o) : log(o.log), val(o.val + 5)
        { log->record('N', cxlog::MOVE, this, &o); }
    constexpr ~NTL() { log->record('N', cxlog::DTOR, this); }
};

constexpr int test_nontrivial_log() {
    cxlog log;
    { NTL n(log, 10); NTL m = reloc n; (void)m; }
    return log.match_ops("N(); ~N()");
}
static_assert(test_nontrivial_log() == 1, "move ctor must be elided");

int main()
{
    std::cout << "trivial_move: " << test_trivial() << std::endl;
    std::cout << "nontrivial_move: " << test_nontrivial() << std::endl;
    // cxlog trace
    cxlog log;
    { NTL n(log, 10); NTL m = reloc n; (void)m; }
    std::cout << "trace: "; log.dump(); std::cout << std::endl;
    return 0;
}

////// BUILD SUCCESS
// trivial_move: 7
// nontrivial_move: 10
// trace: N() 0x1; ~N() 0x1
