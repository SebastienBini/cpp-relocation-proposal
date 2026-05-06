// virtual-slicing-008: __has_virtual_slicing_function type trait checks
// at compile time. Various positive and negative cases.

#include <iostream>

// Case 1: explicit reloc ctor + virtual dtor → has slicing fn
struct A {
    A();
    A(A reloc);
    virtual ~A() = default;
};
static_assert(__has_virtual_slicing_function(A), "A should have slicing fn");

// Case 2: no virtual dtor → no slicing fn
struct B {
    B();
    B(B reloc);
    ~B() = default;
};
static_assert(!__has_virtual_slicing_function(B), "B has no virtual dtor");

// Case 3: virtual dtor but no explicit reloc ctor, no base with slicing fn
struct C {
    C();
    virtual ~C() = default;
};
static_assert(!__has_virtual_slicing_function(C), "C has no reloc ctor");

// Case 4: inherits from A (base with slicing fn) + explicit reloc ctor
struct D : A {
    D();
    D(D reloc);
};
static_assert(__has_virtual_slicing_function(D), "D inherits from A");

// Case 5: inherits from A but no explicit reloc ctor — Phase 12e says YES
struct E : A {
    E();
};
static_assert(__has_virtual_slicing_function(E), "E inherits slicing fn (Phase 12e)");

// Case 6: deleted reloc ctor + virtual dtor → no slicing fn
struct F {
    F();
    F(F reloc) = delete;
    virtual ~F() = default;
};
static_assert(!__has_virtual_slicing_function(F), "F has deleted reloc ctor");

// Case 7: multi-level inheritance
struct G : D {
    G();
    G(G reloc);
};
static_assert(__has_virtual_slicing_function(G), "G inherits through D");

// Case 8: template
template<typename T>
struct Poly {
    T val;
    Poly() : val{} {}
    Poly(Poly reloc) : val{} {}
    virtual ~Poly() = default;
};
static_assert(__has_virtual_slicing_function(Poly<int>), "Poly<int> has slicing fn");
static_assert(__has_virtual_slicing_function(Poly<double>), "Poly<double> has slicing fn");

int main() {
    std::cout << "All static assertions passed." << std::endl;
    return 0;
}

////// BUILD SUCCESS
// All static assertions passed.
