/// Exception handling: field destructor throws BEFORE the target base
/// is constructed (during decomposition).
///
/// Hierarchy:
///   struct Base { snoop b; };  // polymorphic, has reloc ctor
///   struct Derived : Base { snoop d1; ThrowingDtor td; snoop d2; };
///
/// Test: Slice Derived* → Base* where ThrowingDtor::~ThrowingDtor throws
/// during the field-destruction phase of decomposition.
///
/// Decomposition order for Derived → Base (Base is V):
///   Pre-V bases: (none, Base is first)
///   V = Base
///   Post-V bases: (none)
///   Fields: d2 destroyed first, then td, then d1 (reverse decl order)
///
/// Scenario:
///   1. d2 destroyed (OK)
///   2. td's dtor throws
///   3. d1 has NOT been destroyed yet → EH cleanup should destroy it
///   4. Base subobject has NOT been relocated → EH cleanup should destroy it
///   5. Exception propagates to caller

#include <iostream>
#include <memory>
#include <stdexcept>
#include "snoop.h"

static bool arm_throw = false;

struct ThrowingDtor {
    snoop s;
    ThrowingDtor() : s("td") { std::cout << "ThrowingDtor() " << this << std::endl; }
    ThrowingDtor(ThrowingDtor reloc rhs) : s(reloc rhs.s) {
        std::cout << "ThrowingDtor(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~ThrowingDtor() noexcept(false) {
        std::cout << "~ThrowingDtor() " << this << std::endl;
        if (arm_throw) {
            arm_throw = false;  // throw only once
            throw std::runtime_error("dtor-boom-before");
        }
    }
};

struct Base {
    snoop b;
    Base() : b("b") { std::cout << "Base() " << this << std::endl; }
    Base(Base reloc src) : b(reloc src.b) {
        std::cout << "Base(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Base() noexcept(false) = default;
};

struct Derived : Base {
    snoop d1{"d1"};
    ThrowingDtor td;
    snoop d2{"d2"};
    Derived() { std::cout << "Derived() " << this << std::endl; }
    Derived(Derived reloc src)
        : Base(reloc src.base<Base>), d1(reloc src.d1),
          td(reloc src.td), d2(reloc src.d2) {
        std::cout << "Derived(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Derived() noexcept(false) override = default;
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
// Base() 0x2
// d1() 0x3
// td() 0x4
// d2() 0x5
// Derived() 0x2
// ---arm and slice Derived* to Base*---
// ~d2() 0x5
// ~ThrowingDtor() 0x4
// ~td() 0x4
// ~d1() 0x3
// ~b() 0x1
// caught: dtor-boom-before
// ---done---
