#include "snoop.h"

// P2785 §implicit-decomposition [6.3]: C-array element access.
// T elem = getWrapper().arr[1];
// Element [1] is relocated out, elements [0] and [2] are destroyed individually.

struct Wrapper {
    snoop arr[3];
};

Wrapper getWrapper() {
    Wrapper w{snoop{"a"}, snoop{"b"}, snoop{"c"}};
    std::cout << "---" << std::endl;
    return w;
}

int main(int, char**) {
    {
        snoop s = getWrapper().arr[1];
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
// ~c() 0x3
// ~a() 0x1
// --- b alive ---
// ~b() 0x2
// ---
