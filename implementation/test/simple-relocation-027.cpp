/// Virtual base move ctor throws during reloc ctor.
/// B has virtual base VB. When decomposing D and relocating the base B,
/// B's reloc ctor calls VB's move ctor (virtual bases use move, not reloc).
/// VB's move ctor throws. Nothing was fully constructed, so only the
/// VB subobject is destroyed (partially constructed).
/// Uses decomposition (reloc d.base<B>) to force the reloc ctor for B.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

struct VB : public snoop {
    VB() : snoop("VBBase") { std::cout << "VB() " << this << std::endl; }
    VB(VB const& rhs) : snoop(rhs) { std::cout << "VB(const&) " << this << std::endl; }
    VB(VB&& rhs) : snoop(std::move(rhs)) {
        std::cout << "VB(&&) " << this << " <- " << &rhs << std::endl;
        throw std::runtime_error("VB move boom");
    }
    VB(VB reloc rhs) : snoop(reloc rhs.base<snoop>) { std::cout << "VB(reloc) " << this << std::endl; }
    ~VB() { std::cout << "~VB() " << this << std::endl; }
};

struct B : virtual VB {
    snoop bm;
    B() : VB(), bm("bm") { std::cout << "B() " << this << std::endl; }
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
// bm() 0x2
// B() 0x3
// dm() 0x4
// D() 0x3
// ---
// decomp ---
// VBBase(VBBase&&) 0x5 <- 0x1
// VB(&&) 0x5 <- 0x1
// ~VBBase() 0x5
// ~bm() 0x2
// ~VB() 0x1
// ~VBBase() 0x1
// ~dm() 0x4
// caught
// ---
