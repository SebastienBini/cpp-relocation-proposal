// virtual-slicing-004: Ill-formed - user-provided destructor.
// A user-provided destructor body makes the virtual slicing function ill-formed.

#include "snoop.h"

struct Base {
    snoop s1{"s1"};
    snoop s2{"s2"};
    Base() {}
    Base(Base reloc src) : s1(reloc src.s1), s2(reloc src.s2) {}
    virtual ~Base() {} // user-provided body!
};

int main() {
    return 0;
}

////// BUILD FAILURE
// virtual-slicing-004.cpp:11:13: error: virtual slicing function of 'Base' is ill-formed because the destructor is user-provided
//    11 |     virtual ~Base() {} // user-provided body!
//       |             ^
// 1 error generated.
