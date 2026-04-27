/// Data member throws after base succeeds during reloc ctor.
/// D has base B and member m (Thrower). B's reloc succeeds, Thrower's
/// reloc throws. B (already constructed in target) must be destroyed.
/// Uses "return param" for reloc-only types.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

struct B : public snoop {
    B() : snoop("Base") { std::cout << "B() " << this << std::endl; }
    B(B const&) = delete;
    B(B&&) = delete;
    B(B reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "B(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~B() { std::cout << "~B() " << this << std::endl; }
};

struct Thrower : public snoop {
    Thrower() : snoop("thr") { std::cout << "Thrower() " << this << std::endl; }
    Thrower(Thrower const&) = delete;
    Thrower(Thrower&&) = delete;
    Thrower(Thrower reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "Thrower(reloc) " << this << " <- " << rhs.this << std::endl;
        throw std::runtime_error("member boom after base");
    }
    ~Thrower() { std::cout << "~Thrower() " << this << std::endl; }
};

struct D : B {
    Thrower m;
    D() : B() { std::cout << "D() " << this << std::endl; }
    D(D const&) = delete;
    D(D&&) = delete;
    D(D reloc rhs) : B(reloc rhs.base<B>), m(reloc rhs.m) {
        std::cout << "D(reloc) " << this << std::endl;
    }
    ~D() { std::cout << "~D() " << this << std::endl; }
};

D bar(D d) { return d; }

int main(int, char**)
{
    D d;
    std::cout << "---" << std::endl;
    try { D d2 = bar(reloc d); }
    catch (...) { std::cout << "caught" << std::endl; }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// Base() 0x1
// B() 0x1
// thr() 0x2
// Thrower() 0x2
// D() 0x1
// ---
// Base(Base reloc) 0x3 <- 0x1
// B(reloc) 0x3 <- 0x1
// thr(thr reloc) 0x4 <- 0x2
// Thrower(reloc) 0x4 <- 0x2
// ~thr() 0x4
// ~B() 0x3
// ~Base() 0x3
// caught
// ---
