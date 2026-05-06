// virtual-slicing-010: Relocation preserves member data correctly through slicing.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Base {
    snoop bx{"bx"};
    snoop by{"by"};
    Base() { std::cout << "Base() " << this << std::endl; }
    Base(Base reloc src) : bx(reloc src.bx), by(reloc src.by) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Base() = default;
};

struct Derived : Base {
    snoop dz{"dz"};
    snoop dw{"dw"};
    Derived() { std::cout << "Derived() " << this << std::endl; }
    Derived(Derived reloc src) : Base(reloc src.base<Base>), dz(reloc src.dz), dw(reloc src.dw) {
        std::cout << "Derived(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Derived() override = default;
};

static_assert(__has_virtual_slicing_function(Base), "");
static_assert(__has_virtual_slicing_function(Derived), "");

int main() {
    std::cout << "---construct---" << std::endl;
    auto* d = new Derived();
    std::cout << "---relocate full Derived---" << std::endl;
    {
    Derived result = std::reloc_and_uninitialize(d);
    std::cout << "bx=" << result.bx.name << " by=" << result.by.name
              << " dz=" << result.dz.name << " dw=" << result.dw.name << std::endl;
    }
    std::cout << "---slice to Base---" << std::endl;
    auto* d2 = new Derived();
    std::cout << "---" << std::endl;
    Base* bp = d2;
    Base bres = std::reloc_and_uninitialize(bp);
    std::cout << "bx=" << bres.bx.name << " by=" << bres.by.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// bx() 0x1
// by() 0x2
// Base() 0x3
// dz() 0x4
// dw() 0x5
// Derived() 0x3
// ---relocate full Derived---
// bx(bx reloc) 0x6 <- 0x1
// by(by reloc) 0x7 <- 0x2
// Base(reloc) 0x8 <- 0x3
// dz(dz reloc) 0x9 <- 0x4
// dw(dw reloc) 0x10 <- 0x5
// Derived(reloc) 0x8 <- 0x3
// bx=bx by=by dz=dz dw=dw
// ~dw() 0x10
// ~dz() 0x9
// ~by() 0x7
// ~bx() 0x6
// ---slice to Base---
// bx() 0x11
// by() 0x12
// Base() 0x13
// dz() 0x14
// dw() 0x15
// Derived() 0x13
// ---
// ~dw() 0x15
// ~dz() 0x14
// bx(bx reloc) 0x16 <- 0x11
// by(by reloc) 0x17 <- 0x12
// Base(reloc) 0x18 <- 0x13
// bx=bx by=by
// ---end---
// ~by() 0x17
// ~bx() 0x16
