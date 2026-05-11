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
    virtual void print() const { std::cout << "Holder" << std::endl; }
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
    IntHolder ih2 = std::reloc_and_reclaim(ih);
    ih2.print();

    std::cout << "---double holder exact---" << std::endl;
    auto* dh = new DoubleHolder();
    dh->print();
    DoubleHolder dh2 = std::reloc_and_reclaim(dh);
    dh2.print();

    std::cout << "---slice int holder to base---" << std::endl;
    auto* ih3 = new IntHolder();
    Holder* bp = ih3;
    Holder hres = std::reloc_and_reclaim(bp);
    std::cout << "h1=" << hres.h1.name << " h2=" << hres.h2.name << std::endl;
    hres.print();
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---int holder exact---
// h1() 0x1
// h2() 0x2
// Holder()
// iv() 0x3
// iw() 0x4
// IntHolder()
// IntHolder iv=iv
// h1(h1 reloc) 0x5 <- 0x1
// h2(h2 reloc) 0x6 <- 0x2
// Holder(reloc)
// iv(iv reloc) 0x7 <- 0x3
// iw(iw reloc) 0x8 <- 0x4
// IntHolder(reloc)
// IntHolder iv=iv
// ---double holder exact---
// h1() 0x1
// h2() 0x2
// Holder()
// dv() 0x3
// dw() 0x4
// DoubleHolder()
// DoubleHolder dv=dv
// h1(h1 reloc) 0x9 <- 0x1
// h2(h2 reloc) 0x10 <- 0x2
// Holder(reloc)
// dv(dv reloc) 0x11 <- 0x3
// dw(dw reloc) 0x12 <- 0x4
// DoubleHolder(reloc)
// DoubleHolder dv=dv
// ---slice int holder to base---
// h1() 0x1
// h2() 0x2
// Holder()
// iv() 0x3
// iw() 0x4
// IntHolder()
// ~iw() 0x4
// ~iv() 0x3
// h1(h1 reloc) 0x13 <- 0x1
// h2(h2 reloc) 0x14 <- 0x2
// Holder(reloc)
// h1=h1 h2=h2
// Holder
// ---end---
// ~h2() 0x14
// ~h1() 0x13
// ~dw() 0x12
// ~dv() 0x11
// ~h2() 0x10
// ~h1() 0x9
// ~iw() 0x8
// ~iv() 0x7
// ~h2() 0x6
// ~h1() 0x5
