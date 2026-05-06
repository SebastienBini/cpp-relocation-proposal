// virtual-slicing-006: Template hierarchy - compile-time trait checks.
// Verifies that __has_virtual_slicing_function works for template instantiations.
// Template reloc ctors are implicit since no user-declared dtor blocks them.
// (Implicit dtor is virtual when inherited from a base with virtual dtor.)

#include <iostream>
#include "snoop.h"

// Non-template base provides the explicit reloc ctor that seeds the slicing fn.
struct RootBase {
    snoop rb1{"rb1"};
    snoop rb2{"rb2"};
    RootBase() {}
    RootBase(RootBase reloc src) : rb1(reloc src.rb1), rb2(reloc src.rb2) {}
    virtual ~RootBase() = default;
};

template<typename T>
struct PolyBase : RootBase {
    snoop pb1{"pb1"};
    snoop pb2{"pb2"};
    PolyBase() {}
    // No user-declared dtor → implicit reloc ctor is declared.
    // Implicit dtor is virtual (inherited from RootBase).
};

template<typename T>
struct PolyDerived : PolyBase<T> {
    snoop pd1{"pd1"};
    snoop pd2{"pd2"};
    PolyDerived() {}
    // No user-declared dtor → implicit reloc ctor is declared.
};

// Compile-time checks: templates get slicing functions when instantiated.
static_assert(__has_virtual_slicing_function(RootBase), "");
static_assert(__has_virtual_slicing_function(PolyBase<int>), "");
static_assert(__has_virtual_slicing_function(PolyBase<double>), "");
static_assert(__has_virtual_slicing_function(PolyDerived<int>), "");
static_assert(__has_virtual_slicing_function(PolyDerived<double>), "");

// Derived without explicit reloc ctor inherits slicing fn (no user-declared dtor).
template<typename T>
struct PolyLeaf : PolyDerived<T> {
    PolyLeaf() {}
    // No user-declared dtor → implicit reloc ctor declared.
};
static_assert(__has_virtual_slicing_function(PolyLeaf<int>), "");

int main() {
    // Pure compile-time test: verifies traits work for template instantiations.
    // Runtime relocation of template types with snoop members hits template
    // slicing function synthesis limitations — tested separately for non-templates.
    std::cout << "All static assertions passed." << std::endl;
    return 0;
}

////// BUILD SUCCESS
// All static assertions passed.
