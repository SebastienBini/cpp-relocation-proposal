// virtual-slicing-011: Ill-formed - inaccessible base slicing function
// (base has private destructor).

#include "snoop.h"

class PrivateBase {
public:
    snoop s1{"s1"};
    snoop s2{"s2"};
    PrivateBase() {}
    PrivateBase(PrivateBase reloc src) : s1(reloc src.s1), s2(reloc src.s2) {}
private:
    virtual ~PrivateBase() = default;
};

struct Derived : PrivateBase {
    snoop d1{"d1"};
    snoop d2{"d2"};
    Derived() {}
    Derived(Derived reloc src) : PrivateBase(reloc src.base<PrivateBase>),
        d1(reloc src.d1), d2(reloc src.d2) {}
};

int main() {
    return 0;
}

////// BUILD FAILURE
// virtual-slicing-011.cpp:16:8: error: deleted function '~Derived' cannot override a non-deleted function
//    16 | struct Derived : PrivateBase {
//       |        ^
// virtual-slicing-011.cpp:13:13: note: overridden virtual function is here
//    13 |     virtual ~PrivateBase() = default;
//       |             ^
// virtual-slicing-011.cpp:16:18: note: destructor of 'Derived' is implicitly deleted because base class 'PrivateBase' has an inaccessible destructor
//    16 | struct Derived : PrivateBase {
//       |                  ^
// virtual-slicing-011.cpp:16:8: error: virtual slicing function of 'Derived' is ill-formed because the base class 'PrivateBase' has an inaccessible slicing function
//    16 | struct Derived : PrivateBase {
//       |        ^
// virtual-slicing-011.cpp:16:18: error: base class 'PrivateBase' has private destructor
//    16 | struct Derived : PrivateBase {
//       |                  ^~~~~~~~~~~
// virtual-slicing-011.cpp:13:13: note: declared private here
//    13 |     virtual ~PrivateBase() = default;
//       |             ^
// virtual-slicing-011.cpp:20:27: error: attempt to use a deleted function
//    20 |     Derived(Derived reloc src) : PrivateBase(reloc src.base<PrivateBase>),
//       |                           ^
// virtual-slicing-011.cpp:16:18: note: destructor of 'Derived' is implicitly deleted because base class 'PrivateBase' has an inaccessible destructor
//    16 | struct Derived : PrivateBase {
//       |                  ^
// virtual-slicing-011.cpp:16:18: error: base class 'PrivateBase' has private destructor
//    16 | struct Derived : PrivateBase {
//       |                  ^~~~~~~~~~~
// virtual-slicing-011.cpp:13:13: note: declared private here
//    13 |     virtual ~PrivateBase() = default;
//       |             ^
// 5 errors generated.
