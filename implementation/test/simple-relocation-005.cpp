/// All 3 ctors: reloc + move + copy.
/// Chain: main -> bar -> baz, forwarding by value with reloc at each step.
/// A has all three constructors, so it is NOT relocatable-non-movable.
/// The parameter is not early-destructible, so reloc on the param uses move.

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
// snoop(snoop&&) 0x2 <- 0x1
// A(A&&) 0x2 <- 0x1
// baz
// ~A() 0x2
// ~snoop() 0x2
// ---
// ~A() 0x1
// ~snoop() 0x1
