// virtual-slicing-016: No slicing function without virtual dtor.
// std::reloc_and_uninitialize uses the direct reloc ctor path (no vtable dispatch).

#include <iostream>
#include <memory>
#include "snoop.h"

struct NonPoly {
    snoop np1{"np1"};
    snoop np2{"np2"};
    NonPoly() { std::cout << "NonPoly() " << this << std::endl; }
    NonPoly(NonPoly reloc src) : np1(reloc src.np1), np2(reloc src.np2) {
        std::cout << "NonPoly(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~NonPoly() = default;
};

static_assert(!__has_virtual_slicing_function(NonPoly), "");

int main() {
    std::cout << "---construct---" << std::endl;
    NonPoly* p = new NonPoly();
    std::cout << "---relocate---" << std::endl;
    NonPoly result = std::reloc_and_uninitialize(p);
    std::cout << "np1=" << result.np1.name << " np2=" << result.np2.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// np1() 0x1
// np2() 0x2
// NonPoly() 0x1
// ---relocate---
// np1(np1 reloc) 0x3 <- 0x1
// np2(np2 reloc) 0x4 <- 0x2
// NonPoly(reloc) 0x3 <- 0x1
// np1=np1 np2=np2
// ---end---
// ~np2() 0x4
// ~np1() 0x3
