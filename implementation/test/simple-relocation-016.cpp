/// Reloc-assign where RHS is a function return value, relocatable-non-movable.
/// a = make_a() with move deleted — callee-destroy ABI, eliding variant used.
/// Companion to simple-relocation-014 (which has move ctor, caller-destroy).

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

A make_a()
{
    A tmp;
    std::cout << "make_a" << std::endl;
    return tmp;
}

int main(int, char**)
{
    A a;
    std::cout << "---" << std::endl;
    a = make_a();
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// A() 0x1
// ---
// snoop() 0x2
// A() 0x2
// make_a
// snoop& snoop::operator=(snoop reloc) 0x1 <- 0x2
// A::operator=(A reloc) 0x1 <- 0x2
// ---
// ~A() 0x1
// ~snoop() 0x1
