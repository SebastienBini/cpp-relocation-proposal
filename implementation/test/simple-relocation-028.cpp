/// Data member throws after virtual base move succeeds in reloc ctor.
/// B has virtual base VB. VB's move ctor succeeds, but B's data member
/// initialization throws. VB (already moved) must be destroyed.
/// Uses decomposition to force the reloc ctor for B.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

struct VB : public snoop {
    VB() : snoop("VBBase") { std::cout << "VB() " << this << std::endl; }
    VB(VB const& rhs) : snoop(rhs) { std::cout << "VB(const&) " << this << std::endl; }
    VB(VB&& rhs) : snoop(std::move(rhs)) {
        std::cout << "VB(&&) " << this << " <- " << &rhs << std::endl;
    }
    VB(VB reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "VB(reloc) " << this << std::endl; }
    ~VB() { std::cout << "~VB() " << this << std::endl; }
};

struct Thrower : public snoop {
    Thrower() : snoop("thr") { std::cout << "Thrower() " << this << std::endl; }
    Thrower(Thrower const&) = delete;
    Thrower(Thrower&&) = delete;
    Thrower(Thrower reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "Thrower(reloc) " << this << " <- " << rhs.this << std::endl;
        throw std::runtime_error("member boom after VB");
    }
    ~Thrower() { std::cout << "~Thrower() " << this << std::endl; }
};

struct B : virtual VB {
    Thrower bm;
    B() : VB() { std::cout << "B() " << this << std::endl; }
    B(B const&) = delete;
    B(B&&) = delete;
    B(B reloc rhs) : VB(std::move(rhs.base<VB>)), bm(reloc rhs.bm) {
        std::cout << "B(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~B() { std::cout << "~B() " << this << std::endl; }
};

struct D : B {
    snoop dm;
    D() : dm("dm") { std::cout << "D() " << this << std::endl; }
    D(D const&) = delete;
    D(D&&) = delete;
    D(D reloc rhs) : VB(std::move(rhs.base<VB>)), B(reloc rhs.base<B>), dm(reloc rhs.dm) {
        std::cout << "D(reloc) " << this << std::endl;
    }
    ~D() { std::cout << "~D() " << this << std::endl; }
    friend void decomp(D reloc d);
};

void sink_b(B b) { std::cout << "sink_b" << std::endl; }

void decomp(D reloc d)
{
    std::cout << "decomp ---" << std::endl;
    sink_b(reloc d.base<B>);
    std::cout << "decomp ---" << std::endl;
}

int main(int, char**)
{
    D d;
    std::cout << "---" << std::endl;
    try { decomp(reloc d); }
    catch (...) { std::cout << "caught" << std::endl; }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// VBBase() 0x1
// VB() 0x1
// thr() 0x2
// Thrower() 0x2
// B() 0x3
// dm() 0x4
// D() 0x3
// ---
// decomp ---
// VBBase(VBBase&&) 0x5 <- 0x1
// VB(&&) 0x5 <- 0x1
// thr(thr reloc) 0x6 <- 0x2
// Thrower(reloc) 0x6 <- 0x2
// ~thr() 0x6
// ~VB() 0x5
// ~VBBase() 0x5
// ~VB() 0x1
// ~VBBase() 0x1
// ~dm() 0x4
// caught
// ---
