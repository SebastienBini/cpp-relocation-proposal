#include "snoop.h"

// P2785 §implicit-decomposition [6.1]: derived-to-base cast.
// B b = getD();  where D publicly derives from B.
// The base subobject B is relocated out of the temporary D.
// D's own data members are destroyed individually (not via complete dtor).

struct Base {
    snoop x;
};

struct Derived : Base {
    snoop extra;
};

Derived getDerived() {
    Derived d{snoop{"base"}, snoop{"extra"}};
    std::cout << "--- d is constructed" << std::endl;
    return d;
}

int main(int, char**) {
    std::cout << "---" << std::endl;
    {
        Base b = getDerived();
        std::cout << "--- b alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    {
        Derived d{snoop{"base"}, snoop{"extra"}};
        Base b = reloc d;
        std::cout << "--- b alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---
// base() 0x1
// extra() 0x2
// --- d is constructed
// ~extra() 0x2
// --- b alive ---
// ~base() 0x1
// ---
// base() 0x3
// extra() 0x4
// ~extra() 0x4
// --- b alive ---
// ~base() 0x3
// ---
