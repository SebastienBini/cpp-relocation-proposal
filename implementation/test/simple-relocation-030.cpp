/// Second base throws during reloc ctor with two bases.
/// D has bases B1, B2. B1's reloc ctor succeeds, B2's throws.
/// B1 (already constructed in the target) must be destroyed.
/// Uses "return param" for reloc-only types.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

struct B1 : public snoop {
    B1() : snoop("B1Base") { std::cout << "B1() " << this << std::endl; }
    B1(B1 const&) = delete;
    B1(B1&&) = delete;
    B1(B1 reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "B1(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~B1() { std::cout << "~B1() " << this << std::endl; }
};

struct B2 : public snoop {
    B2() : snoop("B2Base") { std::cout << "B2() " << this << std::endl; }
    B2(B2 const&) = delete;
    B2(B2&&) = delete;
    B2(B2 reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "B2(B2 reloc) " << this << " <- " << rhs.this << std::endl;
        throw std::runtime_error("B2 boom");
    }
    ~B2() { std::cout << "~B2() " << this << std::endl; }
};

struct D : B1, B2 {
    snoop dm;
    D() : dm("dm") { std::cout << "D() " << this << std::endl; }
    D(D const&) = delete;
    D(D&&) = delete;
    D(D reloc rhs) : B1(reloc rhs.base<B1>), B2(reloc rhs.base<B2>), dm(reloc rhs.dm) {
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
// B1Base() 0x1
// B1() 0x1
// B2Base() 0x2
// B2() 0x2
// dm() 0x3
// D() 0x1
// ---
// B1Base(B1Base reloc) 0x4 <- 0x1
// B1(reloc) 0x4 <- 0x1
// B2Base(B2Base reloc) 0x5 <- 0x2
// B2(B2 reloc) 0x5 <- 0x2
// ~B2Base() 0x5
// ~B1() 0x4
// ~B1Base() 0x4
// ~dm() 0x3
// caught
// ---
