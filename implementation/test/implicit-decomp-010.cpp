#include "snoop.h"

// P2785 §implicit-decomposition [6.3]: array in struct with other fields.
// Wrapper has a non-array member before and after the array.

struct Wrapper {
    snoop before;
    snoop arr[3];
    snoop after;
};

Wrapper getWrapper() {
    Wrapper w{snoop{"before"}, {snoop{"a"}, snoop{"b"}, snoop{"c"}}, snoop{"after"}};
    std::cout << "---" << std::endl;
    return w;
}

int main(int, char**) {
    {
        snoop s = getWrapper().arr[0];
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// before() 0x1
// a() 0x2
// b() 0x3
// c() 0x4
// after() 0x5
// ---
// ~after() 0x5
// ~c() 0x4
// ~b() 0x3
// ~before() 0x1
// --- a alive ---
// ~a() 0x2
// ---
