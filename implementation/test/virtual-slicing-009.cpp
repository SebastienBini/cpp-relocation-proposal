// virtual-slicing-009: Multiple inheritance with slicing function.
// Both bases have virtual destructors and explicit reloc ctors.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Poly1 {
    snoop p1a{"p1a"};
    snoop p1b{"p1b"};
    Poly1() { std::cout << "Poly1() " << this << std::endl; }
    Poly1(Poly1 reloc src) : p1a(reloc src.p1a), p1b(reloc src.p1b) {
        std::cout << "Poly1(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Poly1() = default;
};

struct Poly2 {
    snoop p2a{"p2a"};
    snoop p2b{"p2b"};
    Poly2() { std::cout << "Poly2() " << this << std::endl; }
    Poly2(Poly2 reloc src) : p2a(reloc src.p2a), p2b(reloc src.p2b) {
        std::cout << "Poly2(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Poly2() = default;
};

struct Multi : Poly1, Poly2 {
    snoop mc{"mc"};
    snoop md{"md"};
    Multi() { std::cout << "Multi() " << this << std::endl; }
    Multi(Multi reloc src)
        : Poly1(reloc src.base<Poly1>), Poly2(reloc src.base<Poly2>),
          mc(reloc src.mc), md(reloc src.md) {
        std::cout << "Multi(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Multi() override = default;
};

static_assert(__has_virtual_slicing_function(Multi), "");

int main() {
    std::cout << "---construct---" << std::endl;
    Multi* m = new Multi();
    std::cout << "---relocate exact type---" << std::endl;
    {
        Multi result = std::reloc_and_uninitialize(m);
        std::cout << "mc=" << result.mc.name << " md=" << result.md.name << std::endl;
    }
    std::cout << "---slice to Poly1---" << std::endl;
    Multi* m2 = new Multi();
    std::cout << "---" << std::endl;
    Poly1* p1 = m2;
    Poly1 p1result = std::reloc_and_uninitialize(p1);
    std::cout << "p1a=" << p1result.p1a.name << " p1b=" << p1result.p1b.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// p1a() 0x1
// p1b() 0x2
// Poly1() 0x3
// p2a() 0x4
// p2b() 0x5
// Poly2() 0x6
// mc() 0x7
// md() 0x8
// Multi() 0x3
// ---relocate exact type---
// p1a(p1a reloc) 0x9 <- 0x1
// p1b(p1b reloc) 0x10 <- 0x2
// Poly1(reloc) 0x11 <- 0x3
// p2a(p2a reloc) 0x12 <- 0x4
// p2b(p2b reloc) 0x13 <- 0x5
// Poly2(reloc) 0x14 <- 0x6
// mc(mc reloc) 0x15 <- 0x7
// md(md reloc) 0x16 <- 0x8
// Multi(reloc) 0x11 <- 0x3
// mc=mc md=md
// ~md() 0x16
// ~mc() 0x15
// ~p2b() 0x13
// ~p2a() 0x12
// ~p1b() 0x10
// ~p1a() 0x9
// ---slice to Poly1---
// p1a() 0x17
// p1b() 0x18
// Poly1() 0x19
// p2a() 0x20
// p2b() 0x21
// Poly2() 0x22
// mc() 0x23
// md() 0x24
// Multi() 0x19
// ---
// ~md() 0x24
// ~mc() 0x23
// ~p2b() 0x21
// ~p2a() 0x20
// p1a(p1a reloc) 0x25 <- 0x17
// p1b(p1b reloc) 0x26 <- 0x18
// Poly1(reloc) 0x27 <- 0x19
// p1a=p1a p1b=p1b
// ---end---
// ~p1b() 0x26
// ~p1a() 0x25
