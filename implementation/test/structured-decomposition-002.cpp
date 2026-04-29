// P2785 Phase 11 Stage 5 — non-relocated bindings still get destroyed.
//
// Verifies that even when no `reloc` ever fires on the bindings, the
// per-field cleanups for a structured-decomposition still produce exactly
// one destructor call per binding (and not, say, a whole-object dtor that
// happens to also visit the same fields → double-destruction).
#include "snoop.h"

struct Pair {
    snoop x;
    snoop y;
    Pair() : x("X"), y("Y") {}
};

int main(int, char**)
{
    {
        Pair p;
        std::cout << "---" << std::endl;
        auto [a, b] = p;
        std::cout << "..." << std::endl;
    }   // scope exit: ~Y() (b), ~X() (a) [per-field cleanups, LIFO]
        // then: ~Y() (p.y), ~X() (p.x)
    return 0;
}

////// BUILD SUCCESS
// X() 0x1
// Y() 0x2
// ---
// X(X const&); 0x3 <- 0x1
// Y(Y const&); 0x4 <- 0x2
// ...
// ~Y() 0x4
// ~X() 0x3
// ~Y() 0x2
// ~X() 0x1
