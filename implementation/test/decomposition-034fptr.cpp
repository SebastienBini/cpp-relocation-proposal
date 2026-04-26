/// Same as decomposition-034 but decomp is called through a runtime function
/// pointer instead of directly.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct B : public snoop {
    const snoop bx;
    B(int v) : snoop("snoop"), bx("bx") { std::cout << "B() " << this << std::endl; }
    B(B const& rhs) : snoop(rhs), bx(rhs.bx) { std::cout << "B(B const&) " << this << " <- " << &rhs << std::endl; }
    B(B&& rhs) : snoop(std::move(rhs)), bx(rhs.bx) { std::cout << "B(B&&) " << this << " <- " << &rhs << std::endl; }
    B(B reloc rhs) : snoop(reloc rhs.base<snoop>), bx(reloc rhs.bx) { std::cout << "B(B reloc) " << this << " <- " << rhs.this << std::endl; }
    ~B() { std::cout << "~B() " << this << std::endl; }
};

struct D : B {
    const snoop dy;
    D(int b, int d) : B(b), dy("dy") { std::cout << "D() " << this << std::endl; }
    D(D const& rhs) : B(rhs), dy(rhs.dy) { std::cout << "D(D const&) " << this << " <- " << &rhs << std::endl; }
    D(D&& rhs) : B(std::move(rhs)), dy(rhs.dy) { std::cout << "D(D&&) " << this << " <- " << &rhs << std::endl; }
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
    void (*fp)(D) = &decomp;
    const D d(10, 20);
    std::cout << "main ---" << std::endl;
    fp(reloc d);
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
// snoop(snoop&&) 0x4 <- 0x1
// bx(bx const&); 0x5 <- 0x2
// B(B&&) 0x4 <- 0x1
// dy(dy const&); 0x6 <- 0x3
// D(D&&) 0x4 <- 0x1
// decomp ---
// sink_b 0x4
// ~B() 0x4
// ~bx() 0x5
// ~snoop() 0x4
// decomp ---
// ~dy() 0x6
// ~D() 0x1
// ~dy() 0x3
// ~B() 0x1
// ~bx() 0x2
// ~snoop() 0x1
// main ---
