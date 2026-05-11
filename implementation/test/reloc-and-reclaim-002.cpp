// reloc-and-reclaim-002: std::reloc_and_reclaim on a polymorphic type.
// Uses virtual slicing function path for safe polymorphic relocation.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Base {
    snoop b{"b"};
    Base() { std::cout << "Base()" << std::endl; }
    Base(Base reloc src) : b(reloc src.b) {
        std::cout << "Base(reloc)" << std::endl;
    }
    virtual ~Base() = default;
    virtual void print() const { std::cout << "Base b=" << b.name << std::endl; }
};

struct Derived : Base {
    snoop d{"d"};
    Derived() { std::cout << "Derived()" << std::endl; }
    Derived(Derived reloc src) : Base(reloc src.base<Base>), d(reloc src.d) {
        std::cout << "Derived(reloc)" << std::endl;
    }
    ~Derived() override = default;
    void print() const override { std::cout << "Derived b=" << b.name << " d=" << d.name << std::endl; }
};

static_assert(__has_virtual_slicing_function(Base), "");
static_assert(__has_virtual_slicing_function(Derived), "");

int main() {
    std::cout << "---exact type reclaim---" << std::endl;
    auto* p1 = new Derived();
    p1->print();
    Derived result1 = std::reloc_and_reclaim(p1);
    result1.print();

    std::cout << "---slice to base reclaim---" << std::endl;
    auto* p2 = new Derived();
    Base* bp = p2;
    Base result2 = std::reloc_and_reclaim(bp);
    result2.print();
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---exact type reclaim---
// b() 0x1
// Base()
// d() 0x2
// Derived()
// Derived b=b d=d
// b(b reloc) 0x3 <- 0x1
// Base(reloc)
// d(d reloc) 0x4 <- 0x2
// Derived(reloc)
// Derived b=b d=d
// ---slice to base reclaim---
// b() 0x1
// Base()
// d() 0x2
// Derived()
// ~d() 0x2
// b(b reloc) 0x5 <- 0x1
// Base(reloc)
// Base b=b
// ---end---
// ~b() 0x5
// ~d() 0x4
// ~b() 0x3
