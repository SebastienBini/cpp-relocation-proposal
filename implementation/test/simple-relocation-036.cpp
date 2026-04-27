/// Decomposition producing VBA source: SYNTHESIZED C1v reloc ctor throws.
/// Same scenario as test 035 but B has no user-declared reloc ctor —
/// the compiler synthesizes one.  When D is decomposed and B is passed
/// to sink_b, B's synthesized C1v variant is called.  Thrower is armed
/// to throw only on the second relocation, so D's initial reloc succeeds
/// but B's synthesized C1v throws.

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

// B has NO user-declared reloc ctor — the compiler synthesizes one.
// The synthesized C1v variant is exercised when sink_b receives B from
// a decomposed D.
struct B : virtual VB {
    snoop bm;
    Thrower bt;
    B() : bm{"bm"} {}
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
// dm() 0x4
// D() 0x5
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
// ~VB() 0x1
// ~VBBase() 0x1
// ~dm() 0x4
// caught
// ---
