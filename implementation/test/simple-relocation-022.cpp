/// Relocating a type with const data members.
/// The reloc ctor can initialize const members from the source — const
/// doesn't prevent relocation, only reloc-assign.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    const snoop cx;
    int y;
    A(int v) : snoop("snoop"), cx("cx"), y(v) { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs), cx(rhs.cx), y(rhs.y) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>), cx(reloc rhs.cx), y(reloc rhs.y) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    ~A() { std::cout << "~A() " << this << std::endl; }
};

void sink(A a)
{
    std::cout << "sink ---" << &a << std::endl;
    A b = reloc a;
    std::cout << "sink ---" << &b << std::endl;
    reloc b;
    std::cout << "sink ---" << std::endl;
}

int main(int, char**)
{
    A a(20);
    std::cout << "main ---" << std::endl;
    sink(reloc a);
    std::cout << "main ---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// cx() 0x2
// A() 0x1
// main ---
// sink ---0x1
// sink ---0x1
// ~A() 0x1
// ~cx() 0x2
// ~snoop() 0x1
// sink ---
// main ---
