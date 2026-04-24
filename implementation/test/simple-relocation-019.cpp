/// Relocating a type with const data members.
/// The reloc ctor can initialize const members from the source — const
/// doesn't prevent relocation, only reloc-assign.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    const int cx;
    int y;
    A(int c, int v) : snoop("snoop"), cx(c), y(v) { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs), cx(rhs.cx), y(rhs.y) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) : snoop(std::move(rhs)), cx(rhs.cx), y(rhs.y) { std::cout << "A(A&&) " << this << " <- " << &rhs << std::endl; }
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>), cx(reloc rhs.cx), y(reloc rhs.y) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    ~A() { std::cout << "~A() cx=" << cx << " y=" << y << " " << this << std::endl; }
};

void sink(A a)
{
    std::cout << "sink cx=" << a.cx << " y=" << a.y << std::endl;
}

int main(int, char**)
{
    A a(10, 20);
    std::cout << "---" << std::endl;
    sink(reloc a);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// A() 0x1
// ---
// sink cx=10 y=20
// ~A() cx=10 y=20 0x1
// ~snoop() 0x1
// ---
