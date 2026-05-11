// reloc-and-reclaim-001: std::reloc_and_reclaim on a simple non-polymorphic type.
// Allocates with new, relocates + deallocates with reloc_and_reclaim.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Simple {
    snoop s{"s"};
    Simple() { std::cout << "Simple()" << std::endl; }
    Simple(Simple reloc src) : s(reloc src.s) {
        std::cout << "Simple(reloc)" << std::endl;
    }
    ~Simple() { std::cout << "~Simple()" << std::endl; }
};

int main() {
    std::cout << "---begin---" << std::endl;
    auto* p = new Simple();
    std::cout << "---reclaim---" << std::endl;
    Simple result = std::reloc_and_reclaim(p);
    std::cout << "result.s=" << result.s.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---begin---
// s() 0x1
// Simple()
// ---reclaim---
// s(s reloc) 0x2 <- 0x1
// Simple(reloc)
// result.s=s
// ---end---
// ~Simple()
// ~s() 0x2
