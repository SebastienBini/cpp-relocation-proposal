// virtual-slicing-024: Chained virtual inheritance — direct virtual base target.
//
// Hierarchy:
//   struct A {};                  (polymorphic root)
//   struct B : virtual A {};     (has slicing fn)
//   struct C : virtual B {};     (has slicing fn)
//
// Tests:
//   1. Slice C* → B* (direct virtual base target — V-is-virtual shortcut)
//   2. Relocate exact C* → C
//
// NOTE: Slicing C* → A* (base of virtual base) is not yet implemented.
// The V-is-virtual shortcut only handles the case where target == V,
// not target == base-of-V. That would require invoking B's slicing function
// after the move, which is a future enhancement.

#include <iostream>
#include <memory>
#include "snoop.h"

struct A {
    snoop a1{"a1"};
    A() { std::cout << "A() " << this << std::endl; }
    A(A&& src) : a1(std::move(src.a1)) {
        std::cout << "A(move) " << this << " <- " << &src << std::endl;
    }
    A(A reloc src) : a1(reloc src.a1) {
        std::cout << "A(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~A() = default;
};

struct B : virtual A {
    snoop b1{"b1"};
    B() { std::cout << "B() " << this << std::endl; }
    B(B&& src) : A(std::move(src)), b1(std::move(src.b1)) {
        std::cout << "B(move) " << this << " <- " << &src << std::endl;
    }
    B(B reloc src) : A(std::move(src.base<A>)), b1(reloc src.b1) {
        std::cout << "B(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~B() override = default;
};

struct C : virtual B {
    snoop c1{"c1"};
    C() { std::cout << "C() " << this << std::endl; }
    C(C reloc src)
        : A(std::move(src.base<A>))
        , B(std::move(src.base<B>))
        , c1(reloc src.c1) {
        std::cout << "C(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~C() override = default;
};

int main() {
    // Test 1: Slice C* → A (virtual base of virtual base)
    std::cout << "---construct---" << std::endl;
    {
        C* obj1 = new C();
        std::cout << "---slice C* to A*---" << std::endl;
        A* bp = obj1;
        A bres = std::reloc_and_uninitialize(bp);
        std::cout << "a1=" << bres.a1.name << std::endl;
    }

    // Test 2: Slice C* → B* (direct virtual base)
    std::cout << "---construct---" << std::endl;
    {
        C* obj1 = new C();
        std::cout << "---slice C* to B*---" << std::endl;
        B* bp = obj1;
        B bres = std::reloc_and_uninitialize(bp);
        std::cout << "b1=" << bres.b1.name << std::endl;
    }

    // Test 3: Relocate exact C* → C
    std::cout << "---construct---" << std::endl;
    {
        C* obj2 = new C();
        std::cout << "---relocate exact C*---" << std::endl;
        C cres = std::reloc_and_uninitialize(obj2);
        std::cout << "c1=" << cres.c1.name << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// a1() 0x1
// A() 0x2
// b1() 0x3
// B() 0x4
// c1() 0x5
// C() 0x6
// ---slice C* to A*---
// a1(a1&&) 0x7 <- 0x1
// A(move) 0x8 <- 0x2
// ~c1() 0x5
// ~b1() 0x3
// ~a1() 0x1
// a1=a1
// ~a1() 0x7
// ---construct---
// a1() 0x9
// A() 0x10
// b1() 0x11
// B() 0x12
// c1() 0x13
// C() 0x14
// ---slice C* to B*---
// a1(a1&&) 0x15 <- 0x9
// A(move) 0x16 <- 0x10
// b1(b1&&) 0x17 <- 0x11
// B(move) 0x18 <- 0x12
// ~c1() 0x13
// ~b1() 0x11
// ~a1() 0x9
// b1=b1
// ~b1() 0x17
// ~a1() 0x15
// ---construct---
// a1() 0x19
// A() 0x20
// b1() 0x21
// B() 0x22
// c1() 0x23
// C() 0x24
// ---relocate exact C*---
// a1(a1&&) 0x25 <- 0x19
// A(move) 0x26 <- 0x20
// b1(b1&&) 0x27 <- 0x21
// B(move) 0x28 <- 0x22
// c1(c1 reloc) 0x29 <- 0x23
// C(reloc) 0x30 <- 0x24
// ~b1() 0x21
// ~a1() 0x19
// ~a1() 0x19
// c1=c1
// ~c1() 0x29
// ~b1() 0x27
// ~a1() 0x25
