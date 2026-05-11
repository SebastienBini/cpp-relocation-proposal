// virtual-slicing-018: Type-erased polymorphic holder pattern.
// A non-abstract base with a virtual interface, and typed derived classes.

#include <iostream>
#include <memory>
#include "snoop.h"
#include "reloc_uninit_delete.h"

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
    IntHolder ih2 = reloc_uninit_and_delete(ih);
    ih2.print();

    std::cout << "---double holder exact---" << std::endl;
    auto* dh = new DoubleHolder();
    dh->print();
    DoubleHolder dh2 = reloc_uninit_and_delete(dh);
    dh2.print();

    std::cout << "---slice int holder to base---" << std::endl;
    auto* ih3 = new IntHolder();
    Holder* bp = ih3;
    reloc_uninit_and_delete(bp);
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD FAILURE
// virtual-slicing-017.cpp:13:12: error: parameter type 'Holder' is an abstract class
//    13 |     Holder(Holder reloc src) : h1(reloc src.h1), h2(reloc src.h2) {
//       |            ^
// virtual-slicing-017.cpp:17:18: note: unimplemented pure virtual method 'print' in 'Holder'
//    17 |     virtual void print() const = 0;
//       |                  ^
// In file included from virtual-slicing-017.cpp:7:
// /workspace/llvm-project/P2785/implementation/test/reloc_uninit_delete.h:18:21: error: return type 'std::remove_cv_t<Holder>' (aka 'Holder') is an abstract class
//    18 | std::remove_cv_t<T> reloc_uninit_and_delete(T* src) {
//       |                     ^
// virtual-slicing-017.cpp:64:5: note: in instantiation of function template specialization 'reloc_uninit_and_delete<Holder>' requested here
//    64 |     reloc_uninit_and_delete(bp);
//       |     ^
// In file included from virtual-slicing-017.cpp:5:
// In file included from /workspace/llvm-project/build-make/include/c++/v1/memory:983:
// /workspace/llvm-project/build-make/include/c++/v1/__memory/reloc_and_uninitialize.h:25:40: error: return type 'remove_cv_t<Holder>' (aka 'Holder') is an abstract class
//    25 | _LIBCPP_HIDE_FROM_ABI remove_cv_t<_Tp> reloc_and_uninitialize(_Tp* __src) {
//       |                                        ^
// /workspace/llvm-project/P2785/implementation/test/reloc_uninit_delete.h:32:17: note: in instantiation of function template specialization 'std::reloc_and_uninitialize<Holder>' requested here
//    32 |     return std::reloc_and_uninitialize(src);
//       |                 ^
// virtual-slicing-017.cpp:64:5: note: in instantiation of function template specialization 'reloc_uninit_and_delete<Holder>' requested here
//    64 |     reloc_uninit_and_delete(bp);
//       |     ^
// 3 errors generated.
