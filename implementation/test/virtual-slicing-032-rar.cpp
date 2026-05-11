/// Exception handling: pre-V base destructor throws AFTER the target base
/// is successfully constructed (during pre-V base cleanup in decomposition).
///
/// Hierarchy:
///   struct ThrowingBase { ~ThrowingBase() noexcept(false) { throw...; } };
///   struct Target { snoop t; };  // polymorphic, has reloc ctor
///   struct Derived : ThrowingBase, Target { snoop d; };  // has reloc ctor
///
/// Note: ThrowingBase is declared BEFORE Target, making it a pre-V base.
///
/// Decomposition order for Derived → Target (Target is V):
///   Pre-V bases: ThrowingBase (declared before Target)
///   V = Target
///   Post-V bases: (none)
///   Fields: d
///
///   1. Register EH cleanup for pre-V bases (ThrowingBase)
///   2. Destroy fields: d
///   3. Call Target's slicing function → relocates Target to dest (SUCCESS)
///   4. Destroy pre-V bases: ThrowingBase → dtor THROWS
///
/// At step 4, the returned value (dest) IS fully constructed.
/// The exception leaks through reloc_and_reclaim. Per proposal,
/// both source and returned value must be properly handled.

#include <iostream>
#include <memory>
#include <stdexcept>
#include "snoop.h"

static bool arm_throw = false;

struct ThrowingBase {
    snoop s;
    ThrowingBase() : s("tb") { std::cout << "ThrowingBase() " << this << std::endl; }
    ThrowingBase(ThrowingBase reloc rhs) : s(reloc rhs.s) {
        std::cout << "ThrowingBase(reloc) " << this << " <- " << rhs.this << std::endl;
    }
    ~ThrowingBase() noexcept(false) {
        std::cout << "~ThrowingBase() " << this << std::endl;
        if (arm_throw) {
            arm_throw = false;  // throw only once
            throw std::runtime_error("dtor-boom-after");
        }
    }
};

struct Target {
    snoop t;
    Target() : t("t") { std::cout << "Target() " << this << std::endl; }
    Target(Target reloc src) : t(reloc src.t) {
        std::cout << "Target(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Target() noexcept(false) = default;
};

struct Derived : ThrowingBase, Target {
    snoop d;
    Derived() : d("d") { std::cout << "Derived() " << this << std::endl; }
    Derived(Derived reloc src)
        : ThrowingBase(reloc src.base<ThrowingBase>)
        , Target(reloc src.base<Target>)
        , d(reloc src.d) {
        std::cout << "Derived(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Derived() noexcept(false) override = default;
};

int main() {
    std::cout << "---construct---" << std::endl;
    Derived* dp = new Derived();
    std::cout << "---arm and slice Derived* to Target*---" << std::endl;
    arm_throw = true;
    Target* tp = dp;
    try {
        Target tres = std::reloc_and_reclaim(tp);
        std::cout << "ERROR: should not reach here" << std::endl;
    } catch (std::exception const& e) {
        std::cout << "caught: " << e.what() << std::endl;
    }
    std::cout << "---done---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// tb() 0x1
// ThrowingBase() 0x1
// t() 0x2
// Target() 0x3
// d() 0x4
// Derived() 0x3
// ---arm and slice Derived* to Target*---
// ~d() 0x4
// t(t reloc) 0x5 <- 0x2
// Target(reloc) 0x6 <- 0x3
// ~ThrowingBase() 0x1
// ~tb() 0x1
// ~t() 0x5
// caught: dtor-boom-after
// ---done---
