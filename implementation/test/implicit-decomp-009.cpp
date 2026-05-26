#include "snoop.h"

// P2785 §implicit-decomposition [6.3]: C-array element with runtime index.
// T elem = getWrapper().arr[getIndex()];
// The index is evaluated at runtime; the correct element is relocated.

struct Wrapper {
    snoop arr[3];
};

Wrapper getWrapper() {
    Wrapper w{snoop{"a"}, snoop{"b"}, snoop{"c"}};
    std::cout << "---" << std::endl;
    return w;
}

int getIndex() { return 2; }

int main(int, char**) {
    {
        snoop s = getWrapper().arr[getIndex()];
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// a() 0x1
// b() 0x2
// c() 0x3
// ---
// c(c reloc) 0x4 <- 0x3
// ~b() 0x2
// ~a() 0x1
// --- c alive ---
// ~c() 0x4
// ---
