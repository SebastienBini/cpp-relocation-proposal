// virtual-slicing-003: Static type matches dynamic type - no slicing.
// When the static pointer type equals the most-derived type, the slicing
// function just does a direct relocation (tag matches immediately).

#include <iostream>
#include <memory>
#include "snoop.h"

struct Widget {
    snoop w1{"w1"};
    snoop w2{"w2"};
    Widget() { std::cout << "Widget() " << this << std::endl; }
    Widget(Widget reloc src) : w1(reloc src.w1), w2(reloc src.w2) {
        std::cout << "Widget(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~Widget() = default;
};

int main() {
    std::cout << "---construct---" << std::endl;
    Widget* w = new Widget();
    std::cout << "---relocate (exact type)---" << std::endl;
    Widget result = std::reloc_and_uninitialize(w);
    std::cout << "w1=" << result.w1.name << " w2=" << result.w2.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// w1() 0x1
// w2() 0x2
// Widget() 0x3
// ---relocate (exact type)---
// w1(w1 reloc) 0x4 <- 0x1
// w2(w2 reloc) 0x5 <- 0x2
// Widget(reloc) 0x6 <- 0x3
// w1=w1 w2=w2
// ---end---
// ~w2() 0x5
// ~w1() 0x4
