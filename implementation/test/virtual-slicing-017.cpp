// virtual-slicing-018: Type-erased polymorphic holder pattern.
// A non-abstract base with a virtual interface, and typed derived classes.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Holder {
    snoop h1{"h1"};
    snoop h2{"h2"};
    Holder() { std::cout << "Holder()" << std::endl; }
    Holder(Holder reloc src) : h1(reloc src.h1), h2(reloc src.h2) {
        std::cout << "Holder(reloc)" << std::endl;
    }
    virtual ~Holder() = default;
    virtual void print() const = 0;
};

struct IntHolder : Holder {
    snoop iv{"iv"};
    snoop iw{"iw"};
    IntHolder() { std::cout << "IntHolder()" << std::endl; }
    IntHolder(IntHolder reloc src) : Holder(reloc src.base<Holder>),
        iv(reloc src.iv), iw(reloc src.iw) {
        std::cout << "IntHolder(reloc)" << std::endl;
    }
    ~IntHolder() override = default;
    void print() const override { std::cout << "IntHolder iv=" << iv.name << std::endl; }
};

struct DoubleHolder : Holder {
    snoop dv{"dv"};
    snoop dw{"dw"};
    DoubleHolder() { std::cout << "DoubleHolder()" << std::endl; }
    DoubleHolder(DoubleHolder reloc src) : Holder(reloc src.base<Holder>),
        dv(reloc src.dv), dw(reloc src.dw) {
        std::cout << "DoubleHolder(reloc)" << std::endl;
    }
    ~DoubleHolder() override = default;
    void print() const override { std::cout << "DoubleHolder dv=" << dv.name << std::endl; }
};

static_assert(__has_virtual_slicing_function(Holder), "");
static_assert(__has_virtual_slicing_function(IntHolder), "");
static_assert(__has_virtual_slicing_function(DoubleHolder), "");

int main() {
    std::cout << "---int holder exact---" << std::endl;
    auto* ih = new IntHolder();
    ih->print();
    IntHolder ih2 = std::reloc_and_uninitialize(ih);
    ih2.print();

    std::cout << "---double holder exact---" << std::endl;
    auto* dh = new DoubleHolder();
    dh->print();
    DoubleHolder dh2 = std::reloc_and_uninitialize(dh);
    dh2.print();

    std::cout << "---slice int holder to base---" << std::endl;
    auto* ih3 = new IntHolder();
    Holder* bp = ih3;
    std::reloc_and_uninitialize(bp);
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD FAILURE
// virtual-slicing-017.cpp:12:12: error: parameter type 'Holder' is an abstract class
//    12 |     Holder(Holder reloc src) : h1(reloc src.h1), h2(reloc src.h2) {
//       |            ^
// virtual-slicing-017.cpp:16:18: note: unimplemented pure virtual method 'print' in 'Holder'
//    16 |     virtual void print() const = 0;
//       |                  ^
// In file included from virtual-slicing-017.cpp:5:
// In file included from /workspace/llvm-project/build-make/include/c++/v1/memory:982:
// /workspace/llvm-project/build-make/include/c++/v1/__memory/reloc_and_uninitialize.h:25:40: error: return type 'remove_cv_t<Holder>' (aka 'Holder') is an abstract class
//    25 | _LIBCPP_HIDE_FROM_ABI remove_cv_t<_Tp> reloc_and_uninitialize(_Tp* __src) {
//       |                                        ^
// virtual-slicing-017.cpp:63:10: note: in instantiation of function template specialization 'std::reloc_and_uninitialize<Holder>' requested here
//    63 |     std::reloc_and_uninitialize(bp);
//       |          ^
// 2 errors generated.
