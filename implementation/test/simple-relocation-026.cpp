/// Reloc ctor member throws: during the reloc ctor, a data member's
/// initialization throws. The base was already relocated and must be
/// cleaned up.
/// Uses "return param" for a reloc-only type.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

struct Thrower : public snoop {
    Thrower() : snoop("thr") { std::cout << "Thrower() " << this << std::endl; }
    Thrower(Thrower const&) = delete;
    Thrower(Thrower&&) = delete;
    Thrower(Thrower reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "Thrower(reloc) " << this << " <- " << rhs.this << std::endl;
        throw std::runtime_error("member boom");
    }
    ~Thrower() { std::cout << "~Thrower() " << this << std::endl; }
};

struct D : public snoop {
    Thrower m;
    D() : snoop("DBase") { std::cout << "D() " << this << std::endl; }
    D(D const&) = delete;
    D(D&&) = delete;
    D(D reloc rhs) : snoop(reloc rhs.base<snoop>), m(reloc rhs.m) {
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
// DBase() 0x1
// thr() 0x2
// Thrower() 0x2
// D() 0x1
// ---
// DBase(DBase reloc) 0x3 <- 0x1
// thr(thr reloc) 0x4 <- 0x2
// Thrower(reloc) 0x4 <- 0x2
// ~thr() 0x4
// ~DBase() 0x3
// caught
// ---
