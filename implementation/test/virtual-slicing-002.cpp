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
        Base result = std::reloc_and_uninitialize(bp);
        std::cout << "a1=" << result.a1.name << " a2=" << result.a2.name << std::endl;
    }
    std::cout << "---slice to Mid---" << std::endl;
    {
        Leaf* l = new Leaf();
        std::cout << "---" << std::endl;
        Mid* mp = l;
        Mid result = std::reloc_and_uninitialize(mp);
        std::cout << "m1=" << result.m1.name << " m2=" << result.m2.name << std::endl;
    }
    std::cout << "---full Leaf---" << std::endl;
    {
        Leaf* l = new Leaf();
        std::cout << "---" << std::endl;
        Leaf result = std::reloc_and_uninitialize(l);
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
// Leaf_snoop() 0x16
// Mid_snoop0() 0x17
// BBase() 0x18
// a1() 0x19
// a2() 0x20
// Base() 0x21
// Mid_snoop1() 0x22
// m1() 0x23
// m2() 0x24
// Mid() 0x21
// l1() 0x25
// l2() 0x26
// Leaf() 0x21
// ---
// ~l2() 0x26
// ~l1() 0x25
// Mid_snoop0(Mid_snoop0 reloc) 0x27 <- 0x17
// BBase(BBase reloc) 0x28 <- 0x18
// a1(a1 reloc) 0x29 <- 0x19
// a2(a2 reloc) 0x30 <- 0x20
// Base(reloc) 0x31 <- 0x21
// Mid_snoop1(Mid_snoop1 reloc) 0x32 <- 0x22
// m1(m1 reloc) 0x33 <- 0x23
// m2(m2 reloc) 0x34 <- 0x24
// Mid(reloc) 0x31 <- 0x21
// ~Leaf_snoop() 0x16
// m1=m1 m2=m2
// ~m2() 0x34
// ~m1() 0x33
// ~Mid_snoop1() 0x32
// ~a2() 0x30
// ~a1() 0x29
// ~BBase() 0x28
// ~Mid_snoop0() 0x27
// ---full Leaf---
// Leaf_snoop() 0x35
// Mid_snoop0() 0x36
// BBase() 0x37
// a1() 0x38
// a2() 0x39
// Base() 0x40
// Mid_snoop1() 0x41
// m1() 0x42
// m2() 0x43
// Mid() 0x40
// l1() 0x44
// l2() 0x45
// Leaf() 0x40
// ---
// Leaf_snoop(Leaf_snoop reloc) 0x46 <- 0x35
// Mid_snoop0(Mid_snoop0 reloc) 0x47 <- 0x36
// BBase(BBase reloc) 0x48 <- 0x37
// a1(a1 reloc) 0x49 <- 0x38
// a2(a2 reloc) 0x50 <- 0x39
// Base(reloc) 0x51 <- 0x40
// Mid_snoop1(Mid_snoop1 reloc) 0x52 <- 0x41
// m1(m1 reloc) 0x53 <- 0x42
// m2(m2 reloc) 0x54 <- 0x43
// Mid(reloc) 0x51 <- 0x40
// l1(l1 reloc) 0x55 <- 0x44
// l2(l2 reloc) 0x56 <- 0x45
// Leaf(reloc) 0x51 <- 0x40
// l1=l1 l2=l2
// ~l2() 0x56
// ~l1() 0x55
// ~m2() 0x54
// ~m1() 0x53
// ~Mid_snoop1() 0x52
// ~a2() 0x50
// ~a1() 0x49
// ~BBase() 0x48
// ~Mid_snoop0() 0x47
// ~Leaf_snoop() 0x46
// ---end---
