// virtual-slicing-023: Diamond inheritance with virtual base.

#include <iostream>
#include <memory>
#include "snoop.h"
#include "reloc_uninit_delete.h"

struct Base {
    snoop b1{"b1"};
    Base() { std::cout << "Base() " << this << std::endl; }
    Base(Base&& src) : b1(std::move(src.b1)) {
        std::cout << "Base(&&) " << this << " <- " << &src << std::endl;
    }
    Base(Base reloc src) : b1(reloc src.b1) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Base() = default;
};

struct L : public virtual Base {
    snoop l1{"l1"};
    L() { std::cout << "L() " << this << std::endl; }
    L(L reloc src) : Base(std::move(src.base<Base>)), l1(reloc src.l1) {
        std::cout << "L(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~L() override = default;
};

struct R : public virtual Base {
    snoop r1{"r1"};
    R() { std::cout << "R() " << this << std::endl; }
    R(R reloc src) : Base(std::move(src.base<Base>)), r1(reloc src.r1) {
        std::cout << "R(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~R() override = default;
};

struct D : L, R {
    snoop d1{"d1"};
    D() { std::cout << "D() " << this << std::endl; }
    D(D reloc src) : Base(std::move(src.base<Base>)), L(reloc src.base<L>), R(reloc src.base<R>), d1(reloc src.d1) {
        std::cout << "D(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~D() override = default;
};

int main() {
    // Test 1: Slice D* → Base* through L path
    std::cout << "---construct d1---" << std::endl;
    {
        D* obj1 = new D();
        std::cout << "---slice D* to Base* (via L)---" << std::endl;
        Base* bp = static_cast<L*>(obj1);  // L's Base subobject
        Base bres = reloc_uninit_and_delete(bp);
        std::cout << "b1=" << bres.b1.name << std::endl;
    }

    // Test 2: Slice D* → L*
    std::cout << "---construct d2---" << std::endl;
    {
        D* obj2 = new D();
        std::cout << "---slice D* to L*---" << std::endl;
        L* lp = obj2;
        L lres = reloc_uninit_and_delete(lp);
        std::cout << "l1=" << lres.l1.name << std::endl;
    }

    // Test 3: Relocate exact D* → D
    std::cout << "---construct d3---" << std::endl;
    {
        D* obj3 = new D();
        std::cout << "---relocate exact D*---" << std::endl;
        D dres = reloc_uninit_and_delete(obj3);
        std::cout << "d1=" << dres.d1.name << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// ---construct d1---
// b1() 0x1
// Base() 0x2
// l1() 0x3
// L() 0x4
// r1() 0x5
// R() 0x6
// d1() 0x7
// D() 0x4
// ---slice D* to Base* (via L)---
// b1(b1&&) 0x8 <- 0x1
// Base(&&) 0x9 <- 0x2
// ~d1() 0x7
// ~r1() 0x5
// ~l1() 0x3
// ~b1() 0x1
// b1=b1
// ~b1() 0x8
// ---construct d2---
// b1() 0x1
// Base() 0x2
// l1() 0x3
// L() 0x4
// r1() 0x5
// R() 0x6
// d1() 0x7
// D() 0x4
// ---slice D* to L*---
// ~d1() 0x7
// ~r1() 0x5
// b1(b1&&) 0x10 <- 0x1
// Base(&&) 0x11 <- 0x2
// l1(l1 reloc) 0x12 <- 0x3
// L(reloc) 0x13 <- 0x4
// ~b1() 0x1
// l1=l1
// ~l1() 0x12
// ~b1() 0x10
// ---construct d3---
// b1() 0x1
// Base() 0x2
// l1() 0x3
// L() 0x4
// r1() 0x5
// R() 0x6
// d1() 0x7
// D() 0x4
// ---relocate exact D*---
// b1(b1&&) 0x14 <- 0x1
// Base(&&) 0x15 <- 0x2
// b1(b1&&) 0x16 <- 0x1
// Base(&&) 0x17 <- 0x2
// l1(l1 reloc) 0x18 <- 0x3
// L(reloc) 0x19 <- 0x4
// b1(b1&&) 0x20 <- 0x1
// Base(&&) 0x21 <- 0x2
// r1(r1 reloc) 0x16 <- 0x5
// R(reloc) 0x17 <- 0x6
// d1(d1 reloc) 0x21 <- 0x7
// D(reloc) 0x19 <- 0x4
// ~b1() 0x1
// d1=d1
// ~d1() 0x21
// ~r1() 0x16
// ~l1() 0x18
// ~b1() 0x14
