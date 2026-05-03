/// Decomposition producing VBA source: C1v reloc ctor member throws.
/// D has base B (with virtual base VB). When D is decomposed and B is
/// passed to sink_b, B's C1v variant is called (complete, source is
/// VBA — B's virtual base is still alive in the decomposed D, the caller
/// handles it). Thrower is armed to throw only on the second relocation,
/// so D's initial reloc succeeds but B's C1v reloc ctor throws.
/// The dest VB (moved) and dest bm (relocated) must be cleaned up.
/// The source's remaining subobjects (dm, VB) are cleaned up by the caller.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

static bool arm_throw = false;

struct VB : public snoop {
    VB() : snoop("VBBase") { std::cout << "VB() " << this << std::endl; }
    VB(VB const&) = delete;
    VB(VB&& rhs) : snoop(std::move(rhs)) {
        std::cout << "VB(&&) " << this << " <- " << &rhs << std::endl;
    }
    VB(VB reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "VB(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~VB() { std::cout << "~VB() " << this << std::endl; }
};

struct Thrower : public snoop {
    Thrower() : snoop("thr") { std::cout << "Thrower() " << this << std::endl; }
    Thrower(Thrower const&) = delete;
    Thrower(Thrower&&) = delete;
    Thrower(Thrower reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "Thrower(reloc) " << this << " <- " << rhs.this << std::endl;
        if (arm_throw) throw std::runtime_error("boom");
    }
    ~Thrower() { std::cout << "~Thrower() " << this << std::endl; }
};

struct B : virtual VB {
    snoop bm;
    Thrower bt;
    B() : VB(), bm("bm") { std::cout << "B() " << this << std::endl; }
    B(B reloc rhs) : VB(std::move(rhs.base<VB>)), bm(reloc rhs.bm), bt(reloc rhs.bt) {
        std::cout << "B(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~B() { std::cout << "~B() " << this << std::endl; }
};

struct D : B {
    snoop dm;
    D() : dm("dm") { std::cout << "D() " << this << std::endl; }
    D(D reloc rhs) : VB(std::move(rhs.base<VB>)), B(reloc rhs.base<B>), dm(reloc rhs.dm) {
        std::cout << "D(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~D() { std::cout << "~D() " << this << std::endl; }
    friend void decomp(D reloc d);
};

void sink_b(B b) { std::cout << "sink_b" << std::endl; }

void decomp(D reloc d)
{
    std::cout << "decomp ---" << std::endl;
    arm_throw = true;
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
// thr() 0x3
// Thrower() 0x3
// B() 0x4
// dm() 0x5
// D() 0x4
// ---
// decomp ---
// VBBase(VBBase&&) 0x6 <- 0x1
// VB(&&) 0x6 <- 0x1
// bm(bm reloc) 0x7 <- 0x2
// thr(thr reloc) 0x8 <- 0x3
// Thrower(reloc) 0x8 <- 0x3
// ~thr() 0x8
// ~bm() 0x7
// ~VB() 0x6
// ~VBBase() 0x6
// ~dm() 0x5
// ~VB() 0x1
// ~VBBase() 0x1
// caught
// ---
