/// Relocating a const local object.
/// Per P2785 §"reloc-likes-const", relocation disregards cv-qualifiers.
/// 'reloc s' produces a non-const prvalue even when s is declared const.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    A() : snoop("snoop") { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) : snoop(std::move(rhs)) { std::cout << "A(A&&) " << this << " <- " << &rhs << std::endl; }
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    ~A() { std::cout << "~A() " << this << std::endl; }
};

void sink(A a)
{
    std::cout << "sink" << std::endl;
}

int main(int, char**)
{
    const A a;
    std::cout << "---" << std::endl;
    sink(reloc a);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// A() 0x1
// ---
// sink
// ~A() 0x1
// ~snoop() 0x1
// ---
