#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    A() : snoop("snoop") { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) = delete;
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    A& operator=(A const& rhs) { static_cast<snoop&>(*this) = static_cast<snoop const&>(rhs); std::cout << "A::operator=(A const&) " << this << " = " << &rhs << std::endl; return *this; }
    A& operator=(A&& rhs) { static_cast<snoop&>(*this) = rhs; std::cout << "A::operator=(A&&) " << this << " = " << &rhs << std::endl; return *this; }
    A& operator=(A reloc rhs) { static_cast<snoop&>(*this) = reloc rhs.base<snoop>; std::cout << "A::operator=(A reloc) " << this << " = " << rhs.this << std::endl; return *this; }
    ~A() { std::cout << "~A() " << this << std::endl; }
    friend void foo(A reloc obj) {}
};

int main(int, char**)
{
    A a;
    foo(reloc a);
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// A() 0x1
// ~snoop() 0x1
