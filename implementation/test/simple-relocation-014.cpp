/// Reloc-assign where RHS is a function return value (not a CXXRelocExpr).
/// Tests Bug A fix: the moved-from temporary must be destroyed after operator=.
/// a = make_a() — non-eliding variant selected, caller-side cleanup fires.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct A : public snoop {
    A() : snoop("snoop") { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) : snoop(std::move(rhs)) { std::cout << "A(A&&) " << this << " <- " << &rhs << std::endl; }
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    A& operator=(A reloc rhs) { static_cast<snoop&>(*this) = reloc rhs.base<snoop>; std::cout << "A::operator=(A reloc) " << this << " = " << rhs.this << std::endl; return *this; }
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
// snoop(snoop&&) 0x3 <- 0x2
// A(A&&) 0x3 <- 0x2
// snoop& snoop::operator=(snoop reloc) 0x1 = 0x3
// A::operator=(A reloc) 0x1 = 0x3
// ~A() 0x2
// ~snoop() 0x2
// ---
// ~A() 0x1
// ~snoop() 0x1
