// virtual-slicing-025: Non-virtual base target with and without virtual
// inheritance from it.
//
// Hierarchy 1 (no virtual inheritance from target):
//   struct Base {};  (polymorphic, slicing root)
//   struct Mid : Base {};
//   struct Leaf : Mid {};
//   Slice Leaf* → Base* (recursive slicing through non-virtual chain)
//
// Hierarchy 2 (virtual inheritance from target):
//   struct VBase {};  (polymorphic, slicing root)
//   struct VDerived : virtual VBase, Extra {};
//   Slice VDerived* → VBase* (V-is-virtual shortcut, with extra non-virtual base)

#include <iostream>
#include <memory>
#include "snoop.h"

// --- Hierarchy 1: deep non-virtual chain ---
struct Base {
    snoop b1{"b1"};
    Base() { std::cout << "Base() " << this << std::endl; }
    Base(Base reloc src) : b1(reloc src.b1) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Base() = default;
};

struct Mid : Base {
    snoop m1{"m1"};
    Mid() { std::cout << "Mid() " << this << std::endl; }
    Mid(Mid reloc src) : Base(reloc src.base<Base>), m1(reloc src.m1) {
        std::cout << "Mid(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Mid() override = default;
};

struct Leaf : Mid {
    snoop lf1{"lf1"};
    snoop lf2{"lf2"};
    Leaf() { std::cout << "Leaf() " << this << std::endl; }
    Leaf(Leaf reloc src)
        : Mid(reloc src.base<Mid>), lf1(reloc src.lf1), lf2(reloc src.lf2) {
        std::cout << "Leaf(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Leaf() override = default;
};

// --- Hierarchy 2: virtual inheritance from target + extra NV base ---
struct Extra {
    snoop e1{"e1"};
    Extra() { std::cout << "Extra() " << this << std::endl; }
    ~Extra() { std::cout << "~Extra() " << this << std::endl; }
};

struct VBase {
    snoop vb1{"vb1"};
    VBase() { std::cout << "VBase() " << this << std::endl; }
    VBase(VBase&& src) : vb1(std::move(src.vb1)) {
        std::cout << "VBase(move) " << this << " <- " << &src << std::endl;
    }
    VBase(VBase reloc src) : vb1(reloc src.vb1) {
        std::cout << "VBase(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~VBase() = default;
};

struct VDerived : Extra, virtual VBase {
    snoop vd1{"vd1"};
    VDerived() { std::cout << "VDerived() " << this << std::endl; }
    VDerived(VDerived reloc src)
        : VBase(std::move(src.base<VBase>))
        , Extra(reloc src.base<Extra>)
        , vd1(reloc src.vd1) {
        std::cout << "VDerived(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~VDerived() override = default;
};

int main() {
    // Test 1: Slice Leaf* → Base* (2-level non-virtual recursion)
    std::cout << "---construct leaf---" << std::endl;
    {
        Leaf* lf = new Leaf();
        std::cout << "---slice Leaf* to Base*---" << std::endl;
        Base* bp = lf;
        Base bres = std::reloc_and_reclaim(bp);
        std::cout << "b1=" << bres.b1.name << std::endl;
    }
    // Test 2: Slice Leaf* → Mid* (1-level non-virtual recursion)
    std::cout << "---construct leaf2---" << std::endl;
    {
        Leaf* lf2 = new Leaf();
        std::cout << "---slice Leaf* to Mid*---" << std::endl;
        Mid* mp = lf2;
        Mid mres = std::reloc_and_reclaim(mp);
        std::cout << "m1=" << mres.m1.name << std::endl;
    }
    // Test 3: Slice VDerived* → VBase* (V-is-virtual shortcut with extra NV base)
    std::cout << "---construct vd---" << std::endl;
    {
        VDerived* vd = new VDerived();
        std::cout << "---slice VDerived* to VBase*---" << std::endl;
        VBase* vbp = vd;
        VBase vbres = std::reloc_and_reclaim(vbp);
        std::cout << "vb1=" << vbres.vb1.name << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// ---construct leaf---
// b1() 0x1
// Base() 0x2
// m1() 0x3
// Mid() 0x2
// lf1() 0x4
// lf2() 0x5
// Leaf() 0x2
// ---slice Leaf* to Base*---
// ~lf2() 0x5
// ~lf1() 0x4
// ~m1() 0x3
// b1(b1 reloc) 0x6 <- 0x1
// Base(reloc) 0x7 <- 0x2
// b1=b1
// ~b1() 0x6
// ---construct leaf2---
// b1() 0x1
// Base() 0x2
// m1() 0x3
// Mid() 0x2
// lf1() 0x4
// lf2() 0x5
// Leaf() 0x2
// ---slice Leaf* to Mid*---
// ~lf2() 0x5
// ~lf1() 0x4
// b1(b1 reloc) 0x8 <- 0x1
// Base(reloc) 0x9 <- 0x2
// m1(m1 reloc) 0x10 <- 0x3
// Mid(reloc) 0x9 <- 0x2
// m1=m1
// ~m1() 0x10
// ~b1() 0x8
// ---construct vd---
// vb1() 0x11
// VBase() 0x4
// e1() 0x1
// Extra() 0x1
// vd1() 0x3
// VDerived() 0x2
// ---slice VDerived* to VBase*---
// vb1(vb1&&) 0x12 <- 0x11
// VBase(move) 0x13 <- 0x4
// ~vd1() 0x3
// ~Extra() 0x1
// ~e1() 0x1
// ~vb1() 0x11
// vb1=vb1
// ~vb1() 0x12
