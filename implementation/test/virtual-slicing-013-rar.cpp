// virtual-slicing-013: Defaulted destructor on derived is NOT user-provided.
// The slicing function is well-formed. Verifies the reloc path works.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Base {
    snoop s1{"s1"};
    snoop s2{"s2"};
    Base() { std::cout << "Base() " << this << std::endl; }
    Base(Base reloc src) : s1(reloc src.s1), s2(reloc src.s2) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Base() = default;
};

struct DerivedDefaulted : Base {
    snoop d1{"d1"};
    snoop d2{"d2"};
    DerivedDefaulted() { std::cout << "DerivedDefaulted() " << this << std::endl; }
    DerivedDefaulted(DerivedDefaulted reloc src)
        : Base(reloc src.base<Base>), d1(reloc src.d1), d2(reloc src.d2) {
        std::cout << "DerivedDefaulted(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~DerivedDefaulted() override = default;  // NOT user-provided
};

static_assert(__has_virtual_slicing_function(DerivedDefaulted), "");

int main() {
    std::cout << "---construct---" << std::endl;
    auto* d = new DerivedDefaulted();
    std::cout << "---relocate---" << std::endl;
    DerivedDefaulted result = std::reloc_and_reclaim(d);
    std::cout << "d1=" << result.d1.name << " d2=" << result.d2.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// s1() 0x1
// s2() 0x2
// Base() 0x3
// d1() 0x4
// d2() 0x5
// DerivedDefaulted() 0x3
// ---relocate---
// s1(s1 reloc) 0x6 <- 0x1
// s2(s2 reloc) 0x7 <- 0x2
// Base(reloc) 0x8 <- 0x3
// d1(d1 reloc) 0x9 <- 0x4
// d2(d2 reloc) 0x10 <- 0x5
// DerivedDefaulted(reloc) 0x8 <- 0x3
// d1=d1 d2=d2
// ---end---
// ~d2() 0x10
// ~d1() 0x9
// ~s2() 0x7
// ~s1() 0x6
