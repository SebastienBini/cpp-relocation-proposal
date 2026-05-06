// virtual-slicing-005: Derived class without explicit reloc ctor or
// user-declared destructor inherits slicing function and gets implicit reloc ctor.
// The implicit reloc ctor is verified via direct local-to-local relocation
// (which benefits from relocation elision — no intermediate ctor call).

#include <iostream>
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

// No explicit reloc ctor and no user-declared destructor.
// The destructor is still virtual (inherited from Base).
// Derived gets an implicit reloc ctor per the spec rules.
struct Derived : Base {
    snoop d1{"d1"};
    snoop d2{"d2"};
    Derived() { std::cout << "Derived() " << this << std::endl; }
    // No explicit reloc ctor here!
    // No user-declared destructor - implicit dtor is virtual (inherited).
};

static_assert(__has_virtual_slicing_function(Base), "");
static_assert(__has_virtual_slicing_function(Derived), "");

Derived foo(Derived d) { return d; }

int main() {
    std::cout << "---construct---" << std::endl;
    Derived d;
    std::cout << "---relocate---" << std::endl;
    Derived d2 = foo(reloc d);
    std::cout << "d2.d1=" << d2.d1.name << " d2.d2=" << d2.d2.name << std::endl;
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
// Derived() 0x3
// ---relocate---
// s1(s1 const&); 0x6 <- 0x1
// s2(s2 const&); 0x7 <- 0x2
// d1(d1&&) 0x8 <- 0x4
// d2(d2&&) 0x9 <- 0x5
// ~d2() 0x5
// ~d1() 0x4
// ~s2() 0x2
// ~s1() 0x1
// d2.d1=d1 d2.d2=d2
// ---end---
// ~d2() 0x9
// ~d1() 0x8
// ~s2() 0x7
// ~s1() 0x6
