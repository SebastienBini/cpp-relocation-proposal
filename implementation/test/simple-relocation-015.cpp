/// Reloc-assign eliding variant chain: both outer and inner operator= use
/// the eliding variant.  a = reloc b where the type has a base with its own
/// reloc-assign operator.  Verifies Bug B fix: UseElidingVariant handles
/// CXXDecomposedBaseExpr so the inner operator= also gets the eliding call.

#include <iostream>
#include <string_view>
#include "snoop.h"

struct Base : public snoop {
    Base() : snoop("snoop") { std::cout << "Base() " << this << std::endl; }
    Base(Base const& rhs) : snoop(rhs) { std::cout << "Base(Base const&) " << this << " <- " << &rhs << std::endl; }
    Base(Base&& rhs) : snoop(std::move(rhs)) { std::cout << "Base(Base&&) " << this << " <- " << &rhs << std::endl; }
    Base(Base reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "Base(Base reloc) " << this << " <- " << rhs.this << std::endl; }
    Base& operator=(Base reloc rhs) { static_cast<snoop&>(*this) = reloc rhs.base<snoop>; std::cout << "Base::operator=(Base reloc) " << this << " <- " << rhs.this << std::endl; return *this; }
    ~Base() { std::cout << "~Base() " << this << std::endl; }
};

struct Derived : Base {
    int dy = 42;
    Derived() { std::cout << "Derived() " << this << std::endl; }
    Derived(Derived const& rhs) : Base(rhs), dy(rhs.dy) { std::cout << "Derived(Derived const&) " << this << " <- " << &rhs << std::endl; }
    Derived(Derived&& rhs) : Base(std::move(rhs)), dy(rhs.dy) { std::cout << "Derived(Derived&&) " << this << " <- " << &rhs << std::endl; }
    Derived(Derived reloc rhs) : Base(reloc rhs.base<Base>), dy(reloc rhs.dy) { std::cout << "Derived(Derived reloc) " << this << " <- " << rhs.this << std::endl; }
    Derived& operator=(Derived reloc rhs) { static_cast<Base&>(*this) = reloc rhs.base<Base>; dy = reloc rhs.dy; std::cout << "Derived::operator=(Derived reloc) " << this << " <- " << rhs.this << std::endl; return *this; }
    ~Derived() { std::cout << "~Derived() " << this << std::endl; }
};

int main(int, char**)
{
    Derived a;
    Derived b;
    std::cout << "---" << std::endl;
    a = reloc b;
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// Base() 0x1
// Derived() 0x1
// snoop() 0x2
// Base() 0x2
// Derived() 0x2
// ---
// snoop& snoop::operator=(snoop reloc) 0x1 <- 0x2
// Base::operator=(Base reloc) 0x1 <- 0x2
// Derived::operator=(Derived reloc) 0x1 <- 0x2
// ---
// ~Derived() 0x1
// ~Base() 0x1
// ~snoop() 0x1
