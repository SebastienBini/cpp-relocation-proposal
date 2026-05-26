#include "snoop.h"

// P2785 §implicit-decomposition-ill-formed: virtual base blocks implicit
// decomposition.  Falls back to move constructor.

struct Base {
    snoop x;
};

struct VDerived : virtual Base {
    snoop extra;
};

VDerived getVDerived() {
    VDerived d;
    d.x = snoop{"base"};
    d.extra = snoop{"extra"};
    std::cout << "--- d is constructed" << std::endl;
    return d;
}

int main(int, char**) {
    std::cout << "---" << std::endl;
    {
        Base b = getVDerived();
        std::cout << "--- b alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---
// snoop() 0x1
// snoop() 0x2
// base() 0x3
// base& base::operator=(base reloc) 0x1 <- 0x3
// extra() 0x4
// extra& extra::operator=(extra reloc) 0x2 <- 0x4
// --- d is constructed
// base(base&&) 0x5 <- 0x1
// ~extra() 0x2
// ~base() 0x1
// --- b alive ---
// ~base() 0x5
// ---
