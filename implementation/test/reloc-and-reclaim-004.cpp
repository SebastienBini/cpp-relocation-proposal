// reloc-and-reclaim-004: std::reloc_and_reclaim with class-specific operator delete.
// Verifies that class-defined deallocation function is used instead of global.

#include <iostream>
#include <memory>
#include "snoop.h"

static bool class_delete_called = false;

struct CustomAlloc {
    snoop s{"ca"};
    CustomAlloc() { std::cout << "CustomAlloc()" << std::endl; }
    CustomAlloc(CustomAlloc reloc src) : s(reloc src.s) {
        std::cout << "CustomAlloc(reloc)" << std::endl;
    }
    ~CustomAlloc() { std::cout << "~CustomAlloc()" << std::endl; }

    static void operator delete(void* ptr) noexcept {
        std::cout << "CustomAlloc::operator delete" << std::endl;
        class_delete_called = true;
        ::operator delete(ptr);
    }
};

int main() {
    std::cout << "---begin---" << std::endl;
    auto* p = new CustomAlloc();
    std::cout << "---reclaim---" << std::endl;
    CustomAlloc result = std::reloc_and_reclaim(p);
    std::cout << "result.s=" << result.s.name << std::endl;
    std::cout << "class_delete_called=" << class_delete_called << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---begin---
// ca() 0x1
// CustomAlloc()
// ---reclaim---
// ca(ca reloc) 0x2 <- 0x1
// CustomAlloc(reloc)
// CustomAlloc::operator delete
// result.s=ca
// class_delete_called=1
// ---end---
// ~CustomAlloc()
// ~ca() 0x2
