// virtual-slicing-002: Multi-level hierarchy slicing.
// Relocating a Leaf* via a Base* slices down to Base.
// Relocating via Mid* slices to Mid. Relocating via Leaf* gives full Leaf.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Base : snoop {
    snoop a1{"a1"};
    snoop a2{"a2"};
    Base() : snoop("BBase") { std::cout << "Base() " << this << std::endl; }
    Base(Base reloc src) : snoop(reloc src.base<snoop>), a1(reloc src.a1), a2(reloc src.a2) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Base() = default;
};

struct Mid : snoopN<0>, Base, snoopN<1> {
    snoop m1{"m1"};
    snoop m2{"m2"};
    Mid() : snoopN<0>{"Mid_snoop0"}, snoopN<1>{"Mid_snoop1"} { std::cout << "Mid() " << this << std::endl; }
    Mid(Mid reloc src) : snoopN<0>{reloc src.base<snoopN<0>>}, Base(reloc src.base<Base>),
            snoopN<1>{reloc src.base<snoopN<1>>}, m1(reloc src.m1), m2(reloc src.m2) {
        std::cout << "Mid(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Mid() override = default;
};

struct Leaf : snoopN<2>, Mid {
    snoop l1{"l1"};
    snoop l2{"l2"};
    Leaf() : snoopN<2>{"Leaf_snoop"} { std::cout << "Leaf() " << this << std::endl; }
    Leaf(Leaf reloc src) : snoopN<2>{reloc src.base<snoopN<2>>}, Mid(reloc src.base<Mid>), l1(reloc src.l1), l2(reloc src.l2) {
        std::cout << "Leaf(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Leaf() override = default;
};

int main() {
    std::cout << "---slice to Base---" << std::endl;
    {
        Leaf* l = new Leaf();
        std::cout << "---" << std::endl;
        Base* bp = l;
        Base result = std::reloc_and_reclaim(bp);
        std::cout << "a1=" << result.a1.name << " a2=" << result.a2.name << std::endl;
    }
    std::cout << "---slice to Mid---" << std::endl;
    {
        Leaf* l = new Leaf();
        std::cout << "---" << std::endl;
        Mid* mp = l;
        Mid result = std::reloc_and_reclaim(mp);
        std::cout << "m1=" << result.m1.name << " m2=" << result.m2.name << std::endl;
    }
    std::cout << "---full Leaf---" << std::endl;
    {
        Leaf* l = new Leaf();
        std::cout << "---" << std::endl;
        Leaf result = std::reloc_and_reclaim(l);
        std::cout << "l1=" << result.l1.name << " l2=" << result.l2.name << std::endl;
    }
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---slice to Base---
// Leaf_snoop() 0x1
// Mid_snoop0() 0x2
// BBase() 0x3
// a1() 0x4
// a2() 0x5
// Base() 0x6
// Mid_snoop1() 0x7
// m1() 0x8
// m2() 0x9
// Mid() 0x6
// l1() 0x10
// l2() 0x11
// Leaf() 0x6
// ---
// ~l2() 0x11
// ~l1() 0x10
// ~m2() 0x9
// ~m1() 0x8
// ~Mid_snoop1() 0x7
// BBase(BBase reloc) 0x12 <- 0x3
// a1(a1 reloc) 0x13 <- 0x4
// a2(a2 reloc) 0x14 <- 0x5
// Base(reloc) 0x15 <- 0x6
// ~Mid_snoop0() 0x2
// ~Leaf_snoop() 0x1
// a1=a1 a2=a2
// ~a2() 0x14
// ~a1() 0x13
// ~BBase() 0x12
// ---slice to Mid---
// Leaf_snoop() 0x1
// Mid_snoop0() 0x2
// BBase() 0x3
// a1() 0x4
// a2() 0x5
// Base() 0x6
// Mid_snoop1() 0x7
// m1() 0x8
// m2() 0x9
// Mid() 0x6
// l1() 0x10
// l2() 0x11
// Leaf() 0x6
// ---
// ~l2() 0x11
// ~l1() 0x10
// Mid_snoop0(Mid_snoop0 reloc) 0x16 <- 0x2
// BBase(BBase reloc) 0x17 <- 0x3
// a1(a1 reloc) 0x18 <- 0x4
// a2(a2 reloc) 0x19 <- 0x5
// Base(reloc) 0x20 <- 0x6
// Mid_snoop1(Mid_snoop1 reloc) 0x21 <- 0x7
// m1(m1 reloc) 0x22 <- 0x8
// m2(m2 reloc) 0x23 <- 0x9
// Mid(reloc) 0x20 <- 0x6
// ~Leaf_snoop() 0x1
// m1=m1 m2=m2
// ~m2() 0x23
// ~m1() 0x22
// ~Mid_snoop1() 0x21
// ~a2() 0x19
// ~a1() 0x18
// ~BBase() 0x17
// ~Mid_snoop0() 0x16
// ---full Leaf---
// Leaf_snoop() 0x1
// Mid_snoop0() 0x2
// BBase() 0x3
// a1() 0x4
// a2() 0x5
// Base() 0x6
// Mid_snoop1() 0x7
// m1() 0x8
// m2() 0x9
// Mid() 0x6
// l1() 0x10
// l2() 0x11
// Leaf() 0x6
// ---
// Leaf_snoop(Leaf_snoop reloc) 0x24 <- 0x1
// Mid_snoop0(Mid_snoop0 reloc) 0x25 <- 0x2
// BBase(BBase reloc) 0x26 <- 0x3
// a1(a1 reloc) 0x27 <- 0x4
// a2(a2 reloc) 0x28 <- 0x5
// Base(reloc) 0x29 <- 0x6
// Mid_snoop1(Mid_snoop1 reloc) 0x30 <- 0x7
// m1(m1 reloc) 0x31 <- 0x8
// m2(m2 reloc) 0x32 <- 0x9
// Mid(reloc) 0x29 <- 0x6
// l1(l1 reloc) 0x33 <- 0x10
// l2(l2 reloc) 0x34 <- 0x11
// Leaf(reloc) 0x29 <- 0x6
// l1=l1 l2=l2
// ~l2() 0x34
// ~l1() 0x33
// ~m2() 0x32
// ~m1() 0x31
// ~Mid_snoop1() 0x30
// ~a2() 0x28
// ~a1() 0x27
// ~BBase() 0x26
// ~Mid_snoop0() 0x25
// ~Leaf_snoop() 0x24
// ---end---
