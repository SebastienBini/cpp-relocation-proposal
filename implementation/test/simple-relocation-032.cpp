/// Synthesized reloc ctor: member's reloc ctor throws.
/// S has no user-declared reloc ctor, so the compiler synthesizes one.
/// The synthesized ctor delegates C1 → C2. When the second member's reloc
/// ctor throws, the first member was already consumed. After the fix,
/// C1 must not double-destroy the consumed members.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

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

// S has a synthesized reloc ctor (no user-declared one).
// The second member throws during relocation.
struct S {
    snoop a;
    Thrower b;
};

S bar(S s) { return s; }

int main(int, char**)
{
    S s{snoop{"a"}, Thrower{}};
    std::cout << "---" << std::endl;
    try { S s2 = bar(reloc s); }
    catch (...) { std::cout << "caught" << std::endl; }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// a() 0x1
// thr() 0x2
// Thrower() 0x2
// ---
// a(a reloc) 0x3 <- 0x1
// thr(thr reloc) 0x4 <- 0x2
// Thrower(reloc) 0x4 <- 0x2
// ~thr() 0x4
// ~a() 0x3
// caught
// ---
