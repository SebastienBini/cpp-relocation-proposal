/// Relocatable non-movable: reloc + copy (move deleted).
/// Chain: main -> bar -> baz, forwarding by value with reloc at each step.
/// Because A is relocatable-non-movable, the parameter is early-destructible.
/// reloc on the param uses the relocation constructor (elided where possible).

#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    A() : snoop("snoop") { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) = delete;
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    ~A() { std::cout << "~A() " << this << std::endl; }
};

void baz(A obj)
{
    std::cout << "baz" << std::endl;
}

void bar(A obj)
{
    std::cout << "bar" << std::endl;
    baz(reloc obj);
}

int main(int, char**)
{
    A a;
    std::cout << "---" << std::endl;
    bar(reloc a);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// A() 0x1
// ---
// bar
// baz
// ~A() 0x1
// ~snoop() 0x1
// ---
