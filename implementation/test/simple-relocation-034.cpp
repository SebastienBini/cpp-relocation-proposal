/// Reloc ctor with virtual base: VB's move ctor throws.
/// S has virtual base VB (whose move ctor throws) and reloc-only members.
/// When VB's move ctor throws, no members have been relocated yet.
/// The source's members must be cleaned up; the partially constructed
/// dest VB must be unwound.
/// Uses "return param" to force a reloc ctor call (argument pass is elided).

#include <iostream>
#include <stdexcept>
#include "snoop.h"

struct VB : public snoop {
    VB() : snoop("VBBase") { std::cout << "VB() " << this << std::endl; }
    VB(VB const&) = delete;
    VB(VB&& rhs) : snoop(std::move(rhs)) {
        std::cout << "VB(&&) " << this << " <- " << &rhs << std::endl;
        throw std::runtime_error("VB move boom");
    }
    VB(VB reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "VB(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~VB() { std::cout << "~VB() " << this << std::endl; }
};

struct RelocOnly : public snoop {
    RelocOnly(std::string_view n) : snoop(n) { std::cout << "RelocOnly() " << this << std::endl; }
    RelocOnly(RelocOnly const&) = delete;
    RelocOnly(RelocOnly&&) = delete;
    RelocOnly(RelocOnly reloc rhs) : snoop(reloc rhs.base<snoop>) {
        std::cout << "RelocOnly(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~RelocOnly() { std::cout << "~RelocOnly() " << this << std::endl; }
};

// S has virtual base VB. VB's move ctor throws before any members are relocated.
struct S : virtual VB {
    RelocOnly a;
    RelocOnly b;
    S() : VB(), a("a"), b("b") { std::cout << "S() " << this << std::endl; }
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
// RelocOnly() 0x2
// b() 0x3
// RelocOnly() 0x3
// S() 0x4
// ---
// VBBase(VBBase&&) 0x5 <- 0x1
// VB(&&) 0x5 <- 0x1
// ~VBBase() 0x5
// ~VB() 0x1
// ~VBBase() 0x1
// ~RelocOnly() 0x3
// ~b() 0x3
// ~RelocOnly() 0x2
// ~a() 0x2
// caught
// ---
