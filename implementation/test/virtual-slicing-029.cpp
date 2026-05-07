/// Exception handling: reloc ctor throws during non-virtual base slicing.
///
/// Hierarchy:
///   struct Base { snoop b; Thrower t; };  // polymorphic, has reloc ctor
///   struct Derived : Base { snoop d1; snoop d2; };  // has reloc ctor
///
/// Test: Slice Derived* → Base* where Base's reloc ctor throws because
/// Thrower's reloc ctor is armed.
///
/// Expected behavior (per proposal):
///   - Derived's slicing function decomposes: destroy d2, d1 first.
///   - Then calls Base's slicing function (match case, uses reloc ctor).
///   - Inside Base's reloc ctor: b is successfully relocated, then t's
///     reloc ctor throws.
///   - Normal ctor unwind destroys dest.b (already relocated to dest).
///   - Source Base subobject: lifetime ended at reloc ctor entry (per
///     proposal: "the source object's lifetime ends when the reloc ctor
///     is entered"). No further cleanup needed for source.
///   - Exception caught in main.

#include <iostream>
#include <memory>
#include <stdexcept>
#include "snoop.h"

static bool arm_throw = false;

struct Thrower {
    snoop s;
    Thrower() : s("thr") { std::cout << "Thrower() " << this << std::endl; }
    Thrower(Thrower reloc rhs) : s(reloc rhs.s) {
        std::cout << "Thrower(reloc) " << this << " <- " << rhs.this << std::endl;
        if (arm_throw) throw std::runtime_error("ctor-boom");
    }
    ~Thrower() { std::cout << "~Thrower() " << this << std::endl; }
};

struct Base {
    snoop b;
    Thrower t;
    Base() : b("b") { std::cout << "Base() " << this << std::endl; }
    Base(Base reloc src) : b(reloc src.b), t(reloc src.t) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Base() = default;
};

struct Derived : Base {
    snoop d1{"d1"};
    snoop d2{"d2"};
    Derived() { std::cout << "Derived() " << this << std::endl; }
    Derived(Derived reloc src)
        : Base(reloc src.base<Base>), d1(reloc src.d1), d2(reloc src.d2) {
        std::cout << "Derived(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Derived() override = default;
};

int main() {
    std::cout << "---construct---" << std::endl;
    Derived* dp = new Derived();
    std::cout << "---arm and slice Derived* to Base*---" << std::endl;
    arm_throw = true;
    Base* bp = dp;
    try {
        Base bres = std::reloc_and_uninitialize(bp);
        std::cout << "ERROR: should not reach here" << std::endl;
    } catch (std::exception const& e) {
        std::cout << "caught: " << e.what() << std::endl;
    }
    std::cout << "---done---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// b() 0x1
// thr() 0x2
// Thrower() 0x2
// Base() 0x3
// d1() 0x4
// d2() 0x5
// Derived() 0x3
// ---arm and slice Derived* to Base*---
// ~d2() 0x5
// ~d1() 0x4
// b(b reloc) 0x6 <- 0x1
// thr(thr reloc) 0x7 <- 0x2
// Thrower(reloc) 0x7 <- 0x2
// ~thr() 0x7
// ~b() 0x6
// caught: ctor-boom
// ---done---
