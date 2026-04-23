/// Relocatable non-movable: return by value through a chain.
/// main -> bar(reloc a) -> bar returns reloc obj -> b receives result.
/// Because A is relocatable-non-movable, relocation constructor is preferred.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    A() : snoop("snoop"), member{"member"} { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs), member(rhs.member) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) = delete;
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>), member(reloc rhs.member) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    ~A() { std::cout << "~A() " << this << std::endl; }
    snoop member;
};

A bar(A obj)
{
    std::cout << "bar" << std::endl;
    return obj;
}

int main(int, char**)
{
    A a;
    std::cout << "---" << std::endl;
    A b = bar(reloc a);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// member() 0x2
// A() 0x1
// ---
// bar
// snoop(snoop reloc) 0x3 <- 0x1
// member(member reloc) 0x4 <- 0x2
// A(A reloc) 0x3 <- 0x1
// ---
// ~A() 0x3
// ~member() 0x4
// ~snoop() 0x3
