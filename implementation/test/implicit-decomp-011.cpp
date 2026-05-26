#include "snoop.h"

// P2785 §implicit-decomposition [6.5]: pointer-to-data-member access.
// T elem = getT().*pmf; where pmf is constexpr.
// The resolved member is relocated out; other members are destroyed.

struct Pair {
    snoop first;
    snoop second;
};

Pair getPair() {
    Pair p{snoop{"first"}, snoop{"second"}};
    std::cout << "---" << std::endl;
    return p;
}


int main(int, char**) {
    {
        constexpr snoop Pair::* pmf = &Pair::first;
        snoop s = getPair().*pmf;
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    {
        constexpr snoop Pair::* pmf = &Pair::second;
        snoop s = getPair().*pmf;
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// first() 0x1
// second() 0x2
// ---
// ~second() 0x2
// --- first alive ---
// ~first() 0x1
// ---
// first() 0x3
// second() 0x4
// ---
// ~first() 0x3
// --- second alive ---
// ~second() 0x4
