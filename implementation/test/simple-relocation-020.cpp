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
    A(A&& rhs) : snoop(std::move(rhs)), cx(rhs.cx), y(rhs.y) { std::cout << "A(A&&) " << this << " <- " << &rhs << std::endl; }
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>), cx(reloc rhs.cx), y(reloc rhs.y) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    ~A() { std::cout << "~A() " << this << std::endl; }
};

A sink(A a)
{
    std::cout << "sink " << &a << std::endl;
    return a;
}

int main(int, char**)
{
    A a(20);
    std::cout << "---" << std::endl;
    A b = sink(reloc a);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// cx() 0x2
// A() 0x1
// ---
// sink 0x1
// snoop(snoop&&) 0x3 <- 0x1
// cx(cx const&); 0x4 <- 0x2
// A(A&&) 0x3 <- 0x1
// ~A() 0x1
// ~cx() 0x2
// ~snoop() 0x1
// ---
// ~A() 0x3
// ~cx() 0x4
// ~snoop() 0x3
