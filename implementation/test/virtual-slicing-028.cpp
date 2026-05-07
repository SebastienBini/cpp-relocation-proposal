// virtual-slicing-028: Ill-formed - base with slicing function not accessible
// from T (private inheritance).
//
// If a base class that has a virtual slicing function is reached through a
// private or protected inheritance path, the slicing function cannot legally
// slice to it, so the definition is ill-formed.

#include "snoop.h"

struct Base {
    snoop s1{"s1"};
    Base() {}
    Base(Base reloc src) : s1(reloc src.s1) {}
    virtual ~Base() = default;
};

struct Mid : private Base {
    snoop m1{"m1"};
    Mid() {}
    Mid(Mid reloc src) : Base(reloc src.base<Base>), m1(reloc src.m1) {}
    ~Mid() override = default;
};

struct Derived : Mid {
    snoop d1{"d1"};
    Derived() {}
    Derived(Derived reloc src) : Mid(reloc src.base<Mid>), d1(reloc src.d1) {}
    ~Derived() override = default;
};

int main() {
    return 0;
}

////// BUILD FAILURE
// virtual-slicing-028.cpp:17:8: error: virtual slicing function of 'Mid' is ill-formed because the base class 'Base' has an inaccessible slicing function
//    17 | struct Mid : private Base {
//       |        ^
// virtual-slicing-028.cpp:24:8: error: virtual slicing function of 'Derived' is ill-formed because the base class 'Base' has an inaccessible slicing function
//    24 | struct Derived : Mid {
//       |        ^
// 2 errors generated.
