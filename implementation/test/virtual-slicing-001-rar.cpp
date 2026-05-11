// virtual-slicing-001: Basic polymorphic slicing via std::reloc_and_reclaim.
// When called with a Base*, the slicing function relocates just the Base
// subobject into the result (proper safe slicing).

#include <iostream>
#include <memory>
#include "snoop.h"

struct Base : snoop {
    snoop s1{"s1"};
    snoop s2{"s2"};
    Base() : snoop{"snoop_Base"} { std::cout << "Base() " << this << std::endl; }
    Base(Base reloc src) : snoop{reloc src.base<snoop>}, s1(reloc src.s1), s2(reloc src.s2) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Base() = default;
};

struct Derived : Base, snoopN<0> {
    snoop d1{"d1"};
    snoop d2{"d2"};
    Derived() : snoopN<0>{"snoop_Derived"} { std::cout << "Derived() " << this << std::endl; }
    Derived(Derived reloc src) : Base(reloc src.base<Base>), snoopN<0>{reloc src.base<snoopN<0>>}, d1(reloc src.d1), d2(reloc src.d2) {
        std::cout << "Derived(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Derived() override = default;
};

int main() {
    std::cout << "---construct---" << std::endl;
    {
        Derived* d = new Derived();
        std::cout << "--- constructed ---" << std::endl;
        Base* bp = d;
        std::cout << "---relocate via base pointer (slices to Base)---" << std::endl;
        Base result = std::reloc_and_reclaim(bp);
        std::cout << "result.s1=" << result.s1.name << " result.s2=" << result.s2.name << std::endl;
    }
    std::cout << "---relocate derived directly (no slicing)---" << std::endl;
    Derived* d2 = new Derived();
    std::cout << "--- constructed ---" << std::endl;
    Derived dresult = std::reloc_and_reclaim(d2);
    std::cout << "dresult.d1=" << dresult.d1.name << " dresult.d2=" << dresult.d2.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// snoop_Base() 0x1
// s1() 0x2
// s2() 0x3
// Base() 0x4
// snoop_Derived() 0x5
// d1() 0x6
// d2() 0x7
// Derived() 0x4
// --- constructed ---
// ---relocate via base pointer (slices to Base)---
// ~d2() 0x7
// ~d1() 0x6
// ~snoop_Derived() 0x5
// snoop_Base(snoop_Base reloc) 0x8 <- 0x1
// s1(s1 reloc) 0x9 <- 0x2
// s2(s2 reloc) 0x10 <- 0x3
// Base(reloc) 0x11 <- 0x4
// result.s1=s1 result.s2=s2
// ~s2() 0x10
// ~s1() 0x9
// ~snoop_Base() 0x8
// ---relocate derived directly (no slicing)---
// snoop_Base() 0x1
// s1() 0x2
// s2() 0x3
// Base() 0x4
// snoop_Derived() 0x5
// d1() 0x6
// d2() 0x7
// Derived() 0x4
// --- constructed ---
// snoop_Base(snoop_Base reloc) 0x12 <- 0x1
// s1(s1 reloc) 0x13 <- 0x2
// s2(s2 reloc) 0x14 <- 0x3
// Base(reloc) 0x15 <- 0x4
// snoop_Derived(snoop_Derived reloc) 0x16 <- 0x5
// d1(d1 reloc) 0x17 <- 0x6
// d2(d2 reloc) 0x18 <- 0x7
// Derived(reloc) 0x15 <- 0x4
// dresult.d1=d1 dresult.d2=d2
// ---end---
// ~d2() 0x18
// ~d1() 0x17
// ~snoop_Derived() 0x16
// ~s2() 0x14
// ~s1() 0x13
// ~snoop_Base() 0x12
