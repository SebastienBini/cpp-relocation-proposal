// virtual-slicing-015: Multi-level implicit reloc ctor chain.
// A -> B -> C, where only A has an explicit reloc ctor. B and C get implicit
// reloc ctors because they have no user-declared dtor (implicit dtor is
// virtual, inherited from A).

#include <iostream>
#include "snoop.h"

struct A {
    snoop a1{"a1"};
    snoop a2{"a2"};
    A() { std::cout << "A() " << this << std::endl; }
    A(A reloc src) : a1(reloc src.a1), a2(reloc src.a2) {
        std::cout << "A(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~A() = default;
};

// B has no explicit reloc ctor or user-declared dtor → gets implicit reloc ctor.
struct B : A {
    snoop b1{"b1"};
    snoop b2{"b2"};
    B() { std::cout << "B() " << this << std::endl; }
};

// C also has no explicit reloc ctor or user-declared dtor.
struct C : B {
    snoop c1{"c1"};
    snoop c2{"c2"};
    C() { std::cout << "C() " << this << std::endl; }
};

static_assert(__has_virtual_slicing_function(A), "");
static_assert(__has_virtual_slicing_function(B), "");
static_assert(__has_virtual_slicing_function(C), "");

int main() {
    return 0;
}

////// BUILD SUCCESS
