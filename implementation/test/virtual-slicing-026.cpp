// virtual-slicing-026: Slice to virtual base (B) when B has a non-virtual
// base (A) that also has a slicing function.
//
// Hierarchy:
//   struct A {};              (polymorphic root with slicing fn)
//   struct B : A {};          (non-virtual inherits from A, has slicing fn)
//   struct C : virtual B {};  (virtual inheritance from B)
//   struct D : C {};          (most-derived)
//
// Tests:
//   1. Slice D* → B* (virtual base target — V-is-virtual shortcut)
//   2. Relocate exact D* → D
//
// NOTE: Slicing D* → A* (base of virtual base B) is not yet supported.
// The V-is-virtual shortcut always produces V itself; slicing further
// to V's base would require chaining V's slicing function post-move.

#include <iostream>
#include <memory>
#include "snoop.h"

struct A {
    snoop a1{"a1"};
    A() { std::cout << "A() " << this << std::endl; }
    A(A reloc src) : a1(reloc src.a1) {
        std::cout << "A(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~A() = default;
};

struct B : A {
    snoop b1{"b1"};
    B() { std::cout << "B() " << this << std::endl; }
    B(B&& src) : A(std::move(src)), b1(std::move(src.b1)) {
        std::cout << "B(move) " << this << " <- " << &src << std::endl;
    }
    B(B reloc src) : A(reloc src.base<A>), b1(reloc src.b1) {
        std::cout << "B(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~B() override = default;
};

struct C : virtual B {
    snoop c1{"c1"};
    C() { std::cout << "C() " << this << std::endl; }
    C(C reloc src)
        : B(std::move(src.base<B>))
        , c1(reloc src.c1) {
        std::cout << "C(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~C() override = default;
};

struct D : C {
    snoop d1{"d1"};
    D() { std::cout << "D() " << this << std::endl; }
    D(D reloc src) : B(std::move(src.base<B>)), C(reloc src.base<C>), d1(reloc src.d1) {
        std::cout << "D(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~D() override = default;
};

int main() {
    // Test 1: Slice D* → B* (virtual base through C, V-is-virtual shortcut)
    std::cout << "---construct d1---" << std::endl;
    {
        D* obj1 = new D();
        std::cout << "---slice D* to B*---" << std::endl;
        B* bp = obj1;
        B bres = std::reloc_and_uninitialize(bp);
        std::cout << "b1=" << bres.b1.name << std::endl;
    }
    // Test 2: Relocate exact D* → D
    std::cout << "---construct d2---" << std::endl;
    {
        D* obj2 = new D();
        std::cout << "---relocate exact D*---" << std::endl;
        D dres = std::reloc_and_uninitialize(obj2);
        std::cout << "d1=" << dres.d1.name << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// ---construct d1---
// a1() 0x1
// A() 0x2
// b1() 0x3
// B() 0x2
// c1() 0x4
// C() 0x5
// d1() 0x6
// D() 0x5
// ---slice D* to B*---
// a1(a1 const&); 0x7 <- 0x1
// b1(b1&&) 0x8 <- 0x3
// B(move) 0x9 <- 0x2
// ~d1() 0x6
// ~c1() 0x4
// ~b1() 0x3
// ~a1() 0x1
// b1=b1
// ~b1() 0x8
// ~a1() 0x7
// ---construct d2---
// a1() 0x10
// A() 0x11
// b1() 0x12
// B() 0x11
// c1() 0x13
// C() 0x14
// d1() 0x15
// D() 0x14
// ---relocate exact D*---
// a1(a1 const&); 0x16 <- 0x10
// b1(b1&&) 0x17 <- 0x12
// B(move) 0x18 <- 0x11
// a1(a1 const&); 0x19 <- 0x10
// b1(b1&&) 0x16 <- 0x12
// B(move) 0x20 <- 0x11
// c1(c1 reloc) 0x21 <- 0x13
// C(reloc) 0x22 <- 0x14
// d1(d1 reloc) 0x20 <- 0x15
// D(reloc) 0x22 <- 0x14
// ~b1() 0x12
// ~a1() 0x10
// d1=d1
// ~d1() 0x20
// ~c1() 0x21
// ~b1() 0x17
// ~b1() 0x16
