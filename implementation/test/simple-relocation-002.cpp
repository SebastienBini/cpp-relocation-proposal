#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    A() : snoop("snoop") { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) : snoop(std::move(rhs)) { std::cout << "A(A&&) " << this << " <- " << &rhs << std::endl; }
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    A& operator=(A const& rhs) { static_cast<snoop&>(*this) = static_cast<snoop const&>(rhs); std::cout << "A::operator=(A const&) " << this << " <- " << &rhs << std::endl; return *this; }
    A& operator=(A&& rhs) { static_cast<snoop&>(*this) = rhs; std::cout << "A::operator=(A&&) " << this << " <- " << &rhs << std::endl; return *this; }
    A& operator=(A reloc rhs) { static_cast<snoop&>(*this) = reloc rhs.base<snoop>; std::cout << "A::operator=(A reloc) " << this << " <- " << rhs.this << std::endl; return *this; }
    ~A() { std::cout << "~A() " << this << std::endl; }
    friend void foo(A reloc obj)
    {
        std::cout << "foo body" << std::endl;
    }
};

int main(int, char**)
{
    std::cout << "main begin" << std::endl;
    A a;
    std::cout << "After A ctor" << std::endl;
    foo(reloc a);
    std::cout << "main end" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// main begin
// snoop() 0x1
// A() 0x1
// After A ctor
// foo body
// ~snoop() 0x1
// main end
