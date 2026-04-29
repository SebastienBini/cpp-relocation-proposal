// Virtual `operator reloc[]` invoked via a base reference.  The static type
// of the source expression is `Base`, so name lookup binds to Base's
// declaration; runtime dispatch then selects Derived's override.  The
// override returns a different (but layout-compatible) tuple-like
// containing snoops constructed differently, so the snoop output proves
// virtual dispatch happened.

#include "snoop.h"

struct Result { snoop a, b; };

struct Base
{
    snoop u;
    snoop v;
    Base() : u("U"), v("V") {}
    virtual ~Base() = default;

    virtual Result operator reloc[]() &&
    {
        std::cout << "Base::op reloc[]" << std::endl;
        return Result{static_cast<snoop&&>(u), static_cast<snoop&&>(v)};
    }
};

struct Derived : Base
{
    snoop w; // extra subobject — destroyed by surrounding scope
    Derived() : Base{}, w("W") {}

    Result operator reloc[]() && override
    {
        std::cout << "Derived::op reloc[]" << std::endl;
        // Forward two of the three snoops; w is left to the destructor.
        return Result{static_cast<snoop&&>(u), static_cast<snoop&&>(w)};
    }
};

int main(int, char**)
{
    {
        Derived d;
        Base& br = d;
        std::cout << "---" << std::endl;
        // Static type of the source expression is Base, so name lookup binds
        // to Base::operator reloc[]() &&; virtual dispatch picks Derived's
        // override at run time.
        auto [a, b] = static_cast<Base&&>(br);
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// U() 0x1
// V() 0x2
// W() 0x3
// ---
// Derived::op reloc[]
// U(U&&) 0x4 <- 0x1
// W(W&&) 0x5 <- 0x3
// ...
// ~W() 0x5
// ~U() 0x4
// ~W() 0x3
// ~V() 0x2
// ~U() 0x1
