// reloc-and-reclaim-005: Step 2 — virtual dtor without slicing function.
// When a class has a virtual dtor but no slicing function (no explicit reloc
// ctor), reloc_and_reclaim must dispatch through the vtable to call the
// most-derived deleting destructor (D0). This test verifies that the derived
// destructor body actually runs even when called through a Base*.

#include <iostream>
#include <memory>

static int derived_dtor_count = 0;

struct Base {
    Base() { std::cout << "Base()" << std::endl; }
    Base(Base &&) { std::cout << "Base(move)" << std::endl; }
    virtual ~Base() { std::cout << "~Base()" << std::endl; }
    // No explicit reloc ctor → no slicing function.
};

struct Derived : Base {
    int payload = 42;
    Derived() { std::cout << "Derived()" << std::endl; }
    Derived(Derived &&o) : Base(std::move(o)), payload(o.payload) {
        std::cout << "Derived(move)" << std::endl;
    }
    ~Derived() override {
        std::cout << "~Derived()" << std::endl;
        ++derived_dtor_count;
    }
    // No explicit reloc ctor → no slicing function.
};

// Confirm no slicing function is present.
static_assert(!__has_virtual_slicing_function(Base), "");
static_assert(!__has_virtual_slicing_function(Derived), "");

int main() {
    std::cout << "---step2 reclaim via Base*---" << std::endl;
    Base* bp = new Derived();
    // reloc_and_reclaim through Base* — must virtually dispatch to Derived's
    // deleting destructor so Derived::~Derived() runs.
    Base result = std::reloc_and_reclaim(bp);
    std::cout << "result ok" << std::endl;

    // The derived destructor must have run exactly once (moved-from source).
    if (derived_dtor_count != 1) {
        std::cout << "FAIL: derived_dtor_count=" << derived_dtor_count
                  << " (expected 1)" << std::endl;
        return 1;
    }

    std::cout << "---step2 uninit via Base*---" << std::endl;
    derived_dtor_count = 0;
    // Allocate but use reloc_and_uninitialize (D1 virtual dispatch).
    Base* bp2 = new Derived();
    Base result2 = std::reloc_and_uninitialize(bp2);
    std::cout << "result2 ok" << std::endl;
    ::operator delete(bp2);
    if (derived_dtor_count != 1) {
        std::cout << "FAIL: derived_dtor_count=" << derived_dtor_count
                  << " (expected 1)" << std::endl;
        return 1;
    }
    // Note: bp2's memory is leaked (uninitialize doesn't dealloc). That's OK.

    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---step2 reclaim via Base*---
// Base()
// Derived()
// Base(move)
// ~Derived()
// ~Base()
// result ok
// ---step2 uninit via Base*---
// Base()
// Derived()
// Base(move)
// ~Derived()
// ~Base()
// result2 ok
// ---end---
// ~Base()
// ~Base()
