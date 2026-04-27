/// Reloc ctor base throws: during the reloc ctor, the base class subobject
/// initialization throws. No subobjects were yet fully constructed, so
/// only the source is cleaned up.
/// Uses "return param" for a reloc-only type.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

struct B : public snoop {
    snoop b;
    B() : snoop("Base"), b{"b"} { std::cout << "B() " << this << std::endl; }
    B(B const&) = delete;
    B(B&&) = delete;
    B(B reloc rhs) : snoop(reloc rhs.base<snoop>), b{reloc rhs.b} {
        std::cout << "B(B reloc) " << this << " <- " << rhs.this << std::endl;
        throw std::runtime_error("base boom");
    }
    ~B() { std::cout << "~B() " << this << std::endl; }
};

struct D : B {
    snoop m;
    D() : B(), m("m") { std::cout << "D() " << this << std::endl; }
    D(D const&) = delete;
    D(D&&) = delete;
    D(D reloc rhs) : B(reloc rhs.base<B>), m(reloc rhs.m) {
        std::cout << "D(D reloc) " << this << " <- " << rhs.this << std::endl;
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
// b() 0x2
// B() 0x1
// m() 0x3
// D() 0x1
// ---
// Base(Base reloc) 0x4 <- 0x1
// b(b reloc) 0x5 <- 0x2
// B(B reloc) 0x4 <- 0x1
// ~b() 0x5
// ~Base() 0x4
// ~m() 0x3
// caught
// ---
