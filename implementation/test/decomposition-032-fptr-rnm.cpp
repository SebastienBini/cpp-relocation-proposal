/// Same as decomposition-032fptrrnm: decomp called through function pointer,
/// with snoop-rnm.h (relocatable, non-movable).

#include <iostream>
#include <string_view>
#include "snoop-rnm.h"

struct B1 : public snoop {
    int b1x;
    B1() : snoop("B1Base"), b1x(0) { std::cout << "B1() " << this << std::endl; }
    B1(B1 const& rhs) : snoop(rhs), b1x(rhs.b1x) { std::cout << "B1(B1 const&) " << this << " <- " << &rhs << std::endl; }
    B1(B1 reloc rhs) : snoop(reloc rhs.base<snoop>), b1x(reloc rhs.b1x) { std::cout << "B1(B1 reloc) " << this << " <- " << rhs.this << std::endl; }
    ~B1() { std::cout << "~B1() " << this << std::endl; }
};

struct B2 : public snoop {
    int b2x;
    B2() : snoop("B2Base"), b2x(0) { std::cout << "B2() " << this << std::endl; }
    B2(B2 const& rhs) : snoop(rhs), b2x(rhs.b2x) { std::cout << "B2(B2 const&) " << this << " <- " << &rhs << std::endl; }
    B2(B2 reloc rhs) : snoop(reloc rhs.base<snoop>), b2x(reloc rhs.b2x) { std::cout << "B2(B2 reloc) " << this << " <- " << rhs.this << std::endl; }
    ~B2() { std::cout << "~B2() " << this << std::endl; }
};

struct D : B1, B2 {
    int dy;
    D() : dy(1) { std::cout << "D() " << this << std::endl; }
    D(D reloc rhs) : B1(reloc rhs.base<B1>), B2(reloc rhs.base<B2>), dy(reloc rhs.dy) { std::cout << "D(D reloc) " << this << " <- " << rhs.this << std::endl; }
    ~D() { std::cout << "~D() " << this << std::endl; }
    friend void decomp(D reloc d);
};

void sink_b1(B1 b)
{
    std::cout << "sink_b1" << std::endl;
}

void decomp(D reloc d)
{
    std::cout << "decomp ---" << std::endl;
    sink_b1(reloc d.base<B1>);
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
// B1Base() 0x1
// B1() 0x1
// B2Base() 0x2
// B2() 0x2
// D() 0x1
// main ---
// decomp ---
// sink_b1
// ~B1() 0x1
// ~B1Base() 0x1
// decomp ---
// ~B2() 0x2
// ~B2Base() 0x2
// main ---
