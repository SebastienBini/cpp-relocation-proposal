// `operator reloc[]` defined in a base class, brought into the derived
// class via a `using` declaration, must be findable by the customized
// decomposition protocol.  The standard's data-members rule forbids
// decomposing a class with members in both itself and a base, so this
// test passes only via the customized route.

#include "snoop.h"

struct Inner { snoop a, b; };

struct Base
{
    snoop u;
    snoop v;
    Base() : u("U"), v("V") {}

    Inner operator reloc[]() &&
    {
        return Inner{static_cast<snoop&&>(u), static_cast<snoop&&>(v)};
    }
};

struct Derived : Base
{
    snoop w;
    Derived() : Base{}, w("W") {}
    using Base::operator reloc[];
};

int main(int, char**)
{
    {
        Derived d;
        std::cout << "---" << std::endl;
        auto [a, b] = static_cast<Derived&&>(d);
        std::cout << "..." << std::endl;
        reloc b;
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// U() 0x1
// V() 0x2
// W() 0x3
// ---
// U(U&&) 0x4 <- 0x1
// V(V&&) 0x5 <- 0x2
// ...
// ~V() 0x5
// ...
// ~U() 0x4
// ~W() 0x3
// ~V() 0x2
// ~U() 0x1
