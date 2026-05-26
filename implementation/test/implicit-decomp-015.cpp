#include "snoop.h"

// P2785 §implicit-decomposition [6.5]: pointer-to-data-member access.
// T elem = getT().*pmf; where pmf is constexpr.
// The resolved member is relocated out; other members are destroyed.

struct Base {
    snoop first{"first"};
    snoop arr[3]{snoop{"a"}, snoop{"b"}, snoop{"c"}};
    snoop second{"second"};
};

struct Aux { snoop aux{"aux"}; };

struct Intermediate : virtual Base { snoop i{"Intermediate"}; };

struct Derived : Intermediate, Aux { snoop d{"Derived"}; };

Derived getDerived() {
    Derived p;
    std::cout << "---" << std::endl;
    return p;
}

int getRuntimeIndex() { return 1; }

int main(int, char**) {
    {
        snoop s = getDerived().arr[1];
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// first() 0x1
// a() 0x2
// b() 0x3
// c() 0x4
// second() 0x5
// Intermediate() 0x6
// aux() 0x7
// Derived() 0x8
// ---
// b(b&&) 0x9 <- 0x3
// ~Derived() 0x8
// ~aux() 0x7
// ~Intermediate() 0x6
// ~second() 0x5
// ~c() 0x4
// ~b() 0x3
// ~a() 0x2
// ~first() 0x1
// --- b alive ---
// ~b() 0x9
