/// Decompose a derived type whose base has a const data member.
/// The base's reloc ctor initializes the const member from the source.
/// Relocation of the whole D calls D(D reloc) which decomposes into
/// B(reloc rhs.base<B>), preserving the const member value.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct B : public snoop {
    const int bx;
    B(int v) : snoop("snoop"), bx(v) { std::cout << "B(" << bx << ") " << this << std::endl; }
    B(B const& rhs) : snoop(rhs), bx(rhs.bx) { std::cout << "B(B const&) " << this << " <- " << &rhs << std::endl; }
    B(B&& rhs) : snoop(std::move(rhs)), bx(rhs.bx) { std::cout << "B(B&&) " << this << " <- " << &rhs << std::endl; }
    B(B reloc rhs) : snoop(reloc rhs.base<snoop>), bx(reloc rhs.bx) { std::cout << "B(B reloc) " << this << " <- " << rhs.this << std::endl; }
    ~B() { std::cout << "~B() bx=" << bx << " " << this << std::endl; }
};

struct D : B {
    int dy;
    D(int b, int d) : B(b), dy(d) { std::cout << "D() " << this << std::endl; }
    D(D const& rhs) : B(rhs), dy(rhs.dy) { std::cout << "D(D const&) " << this << " <- " << &rhs << std::endl; }
    D(D&& rhs) : B(std::move(rhs)), dy(rhs.dy) { std::cout << "D(D&&) " << this << " <- " << &rhs << std::endl; }
    D(D reloc rhs) : B(reloc rhs.base<B>), dy(reloc rhs.dy) { std::cout << "D(D reloc) " << this << " <- " << rhs.this << std::endl; }
    ~D() { std::cout << "~D() dy=" << dy << " " << this << std::endl; }
};

void sink(D d)
{
    std::cout << "sink bx=" << d.bx << " dy=" << d.dy << std::endl;
}

int main(int, char**)
{
    D d(10, 20);
    std::cout << "---" << std::endl;
    sink(reloc d);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// B(10) 0x1
// D() 0x1
// ---
// sink bx=10 dy=20
// ~D() dy=20 0x1
// ~B() bx=10 0x1
// ~snoop() 0x1
// ---
