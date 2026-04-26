/// Same as decomposition-034 but with snoop-rnm.h (relocatable, non-movable).

#include <iostream>
#include <string_view>
#include "snoop-rnm.h"

struct B : public snoop {
    const snoop bx;
    B(int v) : snoop("snoop"), bx("bx") { std::cout << "B() " << this << std::endl; }
    B(B const& rhs) : snoop(rhs), bx(rhs.bx) { std::cout << "B(B const&) " << this << " <- " << &rhs << std::endl; }
    B(B reloc rhs) : snoop(reloc rhs.base<snoop>), bx(reloc rhs.bx) { std::cout << "B(B reloc) " << this << " <- " << rhs.this << std::endl; }
    ~B() { std::cout << "~B() " << this << std::endl; }
};

struct D : B {
    const snoop dy;
    D(int b, int d) : B(b), dy("dy") { std::cout << "D() " << this << std::endl; }
    D(D const& rhs) : B(rhs), dy(rhs.dy) { std::cout << "D(D const&) " << this << " <- " << &rhs << std::endl; }
    D(D reloc rhs) : B(reloc rhs.base<B>), dy(reloc rhs.dy) { std::cout << "D(D reloc) " << this << " <- " << rhs.this << std::endl; }
    ~D() { std::cout << "~D() " << this << std::endl; }
    friend void decomp(D reloc d);
};

void sink_b(B b)
{
    std::cout << "sink_b " << &b << std::endl;
}

void decomp(D reloc d)
{
    std::cout << "decomp ---" << std::endl;
    sink_b(reloc d.base<B>);
    std::cout << "decomp ---" << std::endl;
}

int main(int, char**)
{
    const D d(10, 20);
    std::cout << "main ---" << std::endl;
    decomp(reloc d);
    std::cout << "main ---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// bx() 0x2
// B() 0x1
// dy() 0x3
// D() 0x1
// main ---
// decomp ---
// sink_b 0x1
// ~B() 0x1
// ~bx() 0x2
// ~snoop() 0x1
// decomp ---
// ~dy() 0x3
// main ---
