/// Reloc ctor with virtual base: member's reloc ctor throws.
/// S has virtual base VB and members a (snoop) + b (Thrower).
/// Unlike non-vbase classes, C1 cannot delegate to C2 for virtual bases
/// (IsConstructorDelegationValid rejects classes with virtual bases).
/// Each variant emits its body inline. When b's reloc ctor throws, VB
/// was already moved and a was already relocated — the dest subobjects
/// must be cleaned up without double-destroying source subobjects.
/// Uses "return param" to force a reloc ctor call (argument pass is elided).

#include <iostream>
#include <stdexcept>
#include "snoop.h"

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
        throw std::runtime_error("boom");
    }
    ~Thrower() { std::cout << "~Thrower() " << this << std::endl; }
};

// S has virtual base VB.
// Second member throws during relocation.
struct S : virtual VB {
    snoop a;
    Thrower b;
    S() : VB(), a("a") { std::cout << "S() " << this << std::endl; }
    S(S reloc rhs) : VB(std::move(rhs.base<VB>)), a(reloc rhs.a), b(reloc rhs.b) {
        std::cout << "S(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~S() { std::cout << "~S() " << this << std::endl; }
};

S bar(S s) { return reloc s; }

int main(int, char**)
{
    S s;
    std::cout << "---" << std::endl;
    try { S s2 = bar(reloc s); }
    catch (...) { std::cout << "caught" << std::endl; }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// VBBase() 0x1
// VB() 0x1
// a() 0x2
// thr() 0x3
// Thrower() 0x3
// S() 0x4
// ---
// VBBase(VBBase&&) 0x5 <- 0x1
// VB(&&) 0x5 <- 0x1
// a(a reloc) 0x6 <- 0x2
// thr(thr reloc) 0x7 <- 0x3
// Thrower(reloc) 0x7 <- 0x3
// ~thr() 0x7
// ~a() 0x6
// ~VB() 0x5
// ~VBBase() 0x5
// ~VB() 0x1
// ~VBBase() 0x1
// caught
// ---
