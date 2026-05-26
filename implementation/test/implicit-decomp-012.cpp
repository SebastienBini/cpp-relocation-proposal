#include "snoop.h"

// P2785 §implicit-decomposition [6.5]: pointer-to-data-member access.
// T elem = getT().*pmf; where pmf is constexpr.
// The resolved member is relocated out; other members are destroyed.

struct Pair {
    snoop first;
    snoop second;
};

struct Aux { snoop aux; };

struct Derived : Pair, Aux { snoop d; };

Derived getDerived() {
    Derived p{snoop{"first"}, snoop{"second"}, snoop{"Aux"}, snoop{"Derived"}};
    std::cout << "---" << std::endl;
    return p;
}


int main(int, char**) {
    {
        snoop s = getDerived().first;
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "--- with pointer to data member" << std::endl;
    {
        constexpr snoop Derived::* pmf = &Derived::second;
        snoop s = getDerived().*pmf;
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// first() 0x1
// second() 0x2
// Aux() 0x3
// Derived() 0x4
// ---
// ~Derived() 0x4
// ~Aux() 0x3
// ~second() 0x2
// --- first alive ---
// ~first() 0x1
// --- with pointer to data member
// first() 0x5
// second() 0x6
// Aux() 0x7
// Derived() 0x8
// ---
// ~Derived() 0x8
// ~Aux() 0x7
// ~first() 0x5
// --- second alive ---
// ~second() 0x6
