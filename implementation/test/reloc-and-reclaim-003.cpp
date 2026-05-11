// reloc-and-reclaim-003: std::reloc_and_reclaim on a move-only type (no reloc ctor).
// Falls back to move ctor + dtor, then deallocates.

#include <iostream>
#include <memory>
#include "snoop.h"

struct MoveOnly {
    snoop s{"m"};
    MoveOnly() { std::cout << "MoveOnly()" << std::endl; }
    MoveOnly(MoveOnly const&) = delete;
    MoveOnly(MoveOnly&& rhs) : s(std::move(rhs.s)) {
        std::cout << "MoveOnly(&&)" << std::endl;
    }
    ~MoveOnly() { std::cout << "~MoveOnly()" << std::endl; }
};

int main() {
    std::cout << "---begin---" << std::endl;
    auto* p = new MoveOnly();
    std::cout << "---reclaim---" << std::endl;
    MoveOnly result = std::reloc_and_reclaim(p);
    std::cout << "result.s=" << result.s.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---begin---
// m() 0x1
// MoveOnly()
// ---reclaim---
// m(m&&) 0x2 <- 0x1
// MoveOnly(&&)
// ~MoveOnly()
// ~m() 0x1
// result.s=m
// ---end---
// ~MoveOnly()
// ~m() 0x2
