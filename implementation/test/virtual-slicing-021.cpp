// virtual-slicing-021: Slicing through a virtual base reachable only via a
// non-virtual base. The non-virtual base's base variant returns null (can't
// handle virtual bases), and the complete variant falls back to the
// V-is-virtual shortcut.
//
// Hierarchy: Derived : Mid (non-virtual), Mid : virtual Base
// Slicing Derived* -> Base* should work.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Base {
    snoop b1{"b1"};
    Base() { std::cout << "Base() " << this << std::endl; }
    Base(Base reloc src) : b1(reloc src.b1) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    Base(Base&& src) : b1(std::move(src.b1)) {
        std::cout << "Base(move) " << this << " <- " << &src << std::endl;
    }
    virtual ~Base() = default;
};

struct Mid : virtual Base {
    snoop m1{"m1"};
    Mid() { std::cout << "Mid() " << this << std::endl; }
    Mid(Mid reloc src) : Base(std::move(src.base<Base>)), m1(reloc src.m1) {
        std::cout << "Mid(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Mid() override = default;
};

struct Derived : Mid {
    snoop d1{"d1"};
    Derived() { std::cout << "Derived() " << this << std::endl; }
    Derived(Derived reloc src) : Base(std::move(src.base<Base>)), Mid(reloc src.base<Mid>), d1(reloc src.d1) {
        std::cout << "Derived(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Derived() override = default;
};

int main() {
    std::cout << "---construct---" << std::endl;
    Derived* d = new Derived();
    std::cout << "--- constructed ---" << std::endl;

    Base* bp = d;
    std::cout << "---slice Derived* to Base* via virtual base---" << std::endl;
    Base result = std::reloc_and_uninitialize(bp);
    std::cout << "result.b1=" << result.b1.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// b1() 0x1
// Base() 0x2
// m1() 0x3
// Mid() 0x4
// d1() 0x5
// Derived() 0x4
// --- constructed ---
// ---slice Derived* to Base* via virtual base---
// ~d1() 0x5
// ~m1() 0x3
// b1(b1&&) 0x6 <- 0x1
// Base(move) 0x7 <- 0x2
// ~b1() 0x1
// result.b1=b1
// ---end---
// ~b1() 0x6
