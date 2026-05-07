// virtual-slicing-027: Ill-formed - no eligible relocation, move or copy
// constructor on the class itself.
//
// The virtual slicing function needs to be able to construct U from *this
// (or from a base subobject). If the class has no eligible reloc/move/copy
// ctor, the slicing function definition is ill-formed.

#include "snoop.h"

struct Base {
    snoop s1{"s1"};
    Base() {}
    Base(Base reloc src) : s1(reloc src.s1) {}
    virtual ~Base() = default;
};

struct NoCtors : Base {
    snoop d1{"d1"};
    NoCtors() {}
    NoCtors(NoCtors const&) = delete;
    NoCtors(NoCtors&&) = delete;
    NoCtors(NoCtors reloc) = delete;
    ~NoCtors() override = default;
};

int main() {
    return 0;
}

////// BUILD FAILURE
// virtual-slicing-027.cpp:17:8: error: virtual slicing function of 'NoCtors' is ill-formed because it has no eligible relocation, move, or copy constructor
//    17 | struct NoCtors : Base {
//       |        ^
// 1 error generated.
