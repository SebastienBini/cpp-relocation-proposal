// P2785 Phase 11 Stage 4b end-to-end: `operator reloc[]` protocol with
// per-binding reloc.  S has `operator reloc[]` returning a struct R; the
// hidden VarDecl owning the call result is decomposed-by-reloc, so each
// binding gets a per-field cleanup that `(void)reloc a;` can deactivate.
#include "snoop.h"

struct R {
    snoop x;
    snoop y;
};

struct S {
    snoop u;
    snoop v;
    S() : u("U"), v("V") {}
    R operator reloc[]() {
        return R{snoop("X"), snoop("Y")};
    }
};

int main(int, char**)
{
    {
        S s;
        std::cout << "---" << std::endl;
        auto [a, b] = s;     // copy s into DD; DD.operator reloc[]() -> {X,Y}
        std::cout << "..." << std::endl;
        (void)reloc a;       // discard-reloc on a -> ~X() now, deactivate
        std::cout << "..." << std::endl;
    }   // scope exit: ~Y() (b's per-field cleanup)
        //   then DD's defaulted dtor: ~V() ~U() (DD copy)
        //   then s's defaulted dtor: ~V() ~U() (original)
    return 0;
}

////// BUILD SUCCESS
// U() 0x1
// V() 0x2
// ---
// X() 0x3
// Y() 0x4
// ...
// ~X() 0x3
// ...
// ~Y() 0x4
// ~V() 0x2
// ~U() 0x1
