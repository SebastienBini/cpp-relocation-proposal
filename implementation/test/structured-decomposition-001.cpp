// P2785 Phase 11 Stage 5 — structured decomposition + per-binding reloc.
//
// Verifies that for a structured-decomposition `auto [a, b] = p;` the
// generated DecompositionDecl uses per-field cleanups (Phase 5a infra), so
// that `(void)reloc a;` deactivates only the cleanup for binding `a` and
// leaves binding `b`'s cleanup active for scope exit.  Without Stage 5,
// either `~X()` would be called twice (whole-object cleanup + immediate
// reloc dtor) or `~Y()` would be skipped — both observable bugs.
#include "snoop.h"

struct Pair {
    snoop x;
    snoop y;
    Pair() : x("X"), y("Y") {}
};

int main(int, char**)
{
    {
        Pair p;                       // X(), Y() constructed in p
        std::cout << "---" << std::endl;
        auto [a, b] = p;              // copy-ctor X and Y into the DD
        std::cout << "..." << std::endl;
        (void)reloc a;                // discard-reloc: ~X() now, deactivate
        std::cout << "..." << std::endl;
    }                                 // scope exit: ~Y() (b), ~Y() (p.y), ~X() (p.x)
    return 0;
}

////// BUILD SUCCESS
// X() 0x1
// Y() 0x2
// ---
// X(X const&); 0x3 <- 0x1
// Y(Y const&); 0x4 <- 0x2
// ...
// ~X() 0x3
// ...
// ~Y() 0x4
// ~Y() 0x2
// ~X() 0x1
