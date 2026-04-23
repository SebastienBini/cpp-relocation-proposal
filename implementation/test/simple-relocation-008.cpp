/// All 3 ctors: return by value through a chain.
/// main -> bar(reloc a) -> bar returns reloc obj -> baz receives result.
/// Tests reloc on return + reloc on call argument.

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

A bar(A obj)
{
    std::cout << "bar" << std::endl;
    return reloc obj;
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
// A() 0x1
// ---
// bar
// snoop(snoop&&) 0x2 <- 0x1
// A(A&&) 0x2 <- 0x1
// ~A() 0x1
// ~snoop() 0x1
// ---
// ~A() 0x2
// ~snoop() 0x2
