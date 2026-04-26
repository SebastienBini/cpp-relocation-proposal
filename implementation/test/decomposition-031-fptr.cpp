/// Same as decomposition-031 but decomp is called through a runtime function
/// pointer instead of directly.

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
    D(D&& rhs) : B(std::move(rhs)), dy(rhs.dy) { std::cout << "D(D&&) " << this << " <- " << &rhs << std::endl; }
    D(D reloc rhs) : B(reloc rhs.base<B>), dy(reloc rhs.dy) { std::cout << "D(D reloc) " << this << " <- " << rhs.this << std::endl; }
    ~D() { std::cout << "~D() " << this << std::endl; }
    friend void decomp(D reloc d);
};

void sink_b(B b)
{
    std::cout << "sink_b" << std::endl;
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
    D d;
    std::cout << "main ---" << std::endl;
    fp(reloc d);
    std::cout << "main ---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// B() 0x1
// D() 0x1
// main ---
// snoop(snoop&&) 0x2 <- 0x1
// B(B&&) 0x2 <- 0x1
// D(D&&) 0x2 <- 0x1
// decomp ---
// sink_b
// ~B() 0x2
// ~snoop() 0x2
// decomp ---
// ~D() 0x1
// ~B() 0x1
// ~snoop() 0x1
// main ---
