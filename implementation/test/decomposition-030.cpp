/// Base mem-initializer with reloc: D(D reloc rhs) : B(reloc rhs.base<B>).
/// Tests that the reloc ctor can initialize a base class via relocation
/// in the mem-initializer list, with copy elision of the CXXRelocExpr prvalue.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct B : public snoop {
    int bx;
    B() : snoop("snoop"), bx(0) { std::cout << "B() " << this << std::endl; }
    B(B const& rhs) : snoop(rhs), bx(rhs.bx) { std::cout << "B(B const&) " << this << " <- " << &rhs << std::endl; }
    B(B&& rhs) : snoop(std::move(rhs)), bx(rhs.bx) { std::cout << "B(B&&) " << this << " <- " << &rhs << std::endl; }
    B(B reloc rhs) : snoop(reloc rhs.base<snoop>), bx(reloc rhs.bx) { std::cout << "B(B reloc) " << this << " <- " << rhs.this << std::endl; }
    ~B() { std::cout << "~B() " << this << std::endl; }
};

struct D : B {
    int dy;
    D() : dy(1) { std::cout << "D() " << this << std::endl; }
    D(D reloc rhs) : B(reloc rhs.base<B>), dy(reloc rhs.dy) { std::cout << "D(D reloc) " << this << " <- " << rhs.this << std::endl; }
    ~D() { std::cout << "~D() " << this << std::endl; }
};

void sink(D d)
{
    std::cout << "sink" << std::endl;
}

int main(int, char**)
{
    D d;
    std::cout << "---" << std::endl;
    sink(reloc d);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// B() 0x1
// D() 0x1
// ---
// sink
// ~D() 0x1
// ~B() 0x1
// ~snoop() 0x1
// ---
