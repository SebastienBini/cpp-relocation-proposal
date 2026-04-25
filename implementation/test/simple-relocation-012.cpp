/// All 3 ctors: reloc assignment through a chain.
/// main creates a and b, then a = bar(reloc b) where bar returns reloc obj.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    A() : snoop("snoop") { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) = delete;
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    A& operator=(A reloc rhs) { static_cast<snoop&>(*this) = reloc rhs.base<snoop>; std::cout << "A::operator=(A reloc) " << this << " <- " << rhs.this << std::endl; return *this; }
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
    A b;
    std::cout << "---" << std::endl;
    a = bar(reloc b);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// A() 0x1
// snoop() 0x2
// A() 0x2
// ---
// bar
// snoop(snoop reloc) 0x3 <- 0x2
// A(A reloc) 0x3 <- 0x2
// snoop& snoop::operator=(snoop reloc) 0x1 <- 0x3
// A::operator=(A reloc) 0x1 <- 0x3
// ---
// ~A() 0x1
// ~snoop() 0x1
