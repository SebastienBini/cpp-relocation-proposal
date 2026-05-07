/// Exception handling: move ctor throws during virtual base slicing
/// (complete variant).
///
/// Hierarchy:
///   struct VBase { snoop vb; Thrower t; };  // polymorphic, has reloc ctor
///   struct Extra { snoop e; };              // non-polymorphic
///   struct Derived : Extra, virtual VBase { snoop d; };  // has reloc ctor
///
/// Test: Slice Derived* → VBase* where VBase's move ctor throws.
///
/// In the complete variant, the slicing function:
///   1. Move-constructs VBase subobject from *this into dest.
///   2. Calls Dtor_Complete on *this.
///
/// The complete variant uses the MOVE ctor (not reloc) for the virtual
/// base, because it accesses the subobject through the vbase pointer.
/// Thrower's MOVE ctor is armed to throw.
///
/// If step 1 throws:
///   - Dest is not fully constructed (ctor unwind cleans partial dest).
///   - *this must still be destroyed (proposal: "upon exit through an
///     exception, the this object is destroyed").
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
    }
    Thrower(Thrower&& rhs) : s(std::move(rhs.s)) {
        std::cout << "Thrower(&&) " << this << " <- " << &rhs << std::endl;
        if (arm_throw) throw std::runtime_error("vbase-ctor-boom");
    }
    ~Thrower() { std::cout << "~Thrower() " << this << std::endl; }
};

struct VBase {
    snoop vb;
    Thrower t;
    VBase() : vb("vb") { std::cout << "VBase() " << this << std::endl; }
    VBase(VBase reloc src) : vb(reloc src.vb), t(reloc src.t) {
        std::cout << "VBase(reloc) " << this << " <- " << src.this << std::endl;
    }
    VBase(VBase&& rhs) : vb(std::move(rhs.vb)), t(std::move(rhs.t)) {
        std::cout << "VBase(&&) " << this << " <- " << &rhs << std::endl;
    }
    virtual ~VBase() = default;
};

struct Extra {
    snoop e;
    Extra() : e("e") { std::cout << "Extra() " << this << std::endl; }
    Extra(Extra reloc src) : e(reloc src.e) {
        std::cout << "Extra(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Extra() { std::cout << "~Extra() " << this << std::endl; }
};

struct Derived : Extra, virtual VBase {
    snoop d;
    Derived() : d("d") { std::cout << "Derived() " << this << std::endl; }
    Derived(Derived reloc src)
        : VBase(std::move(src.base<VBase>))
        , Extra(reloc src.base<Extra>)
        , d(reloc src.d) {
        std::cout << "Derived(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~Derived() override = default;
};

int main() {
    std::cout << "---construct---" << std::endl;
    Derived* dp = new Derived();
    std::cout << "---arm and slice Derived* to VBase*---" << std::endl;
    arm_throw = true;
    VBase* vbp = dp;
    try {
        VBase vres = std::reloc_and_uninitialize(vbp);
        std::cout << "ERROR: should not reach here" << std::endl;
    } catch (std::exception const& e) {
        std::cout << "caught: " << e.what() << std::endl;
    }
    std::cout << "---done---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// vb() 0x1
// thr() 0x2
// Thrower() 0x2
// VBase() 0x3
// e() 0x4
// Extra() 0x4
// d() 0x5
// Derived() 0x6
// ---arm and slice Derived* to VBase*---
// vb(vb&&) 0x7 <- 0x1
// thr(thr&&) 0x8 <- 0x2
// Thrower(&&) 0x8 <- 0x2
// ~thr() 0x8
// ~vb() 0x7
// ~d() 0x5
// ~Extra() 0x4
// ~e() 0x4
// ~Thrower() 0x2
// ~thr() 0x2
// ~vb() 0x1
// caught: vbase-ctor-boom
// ---done---
