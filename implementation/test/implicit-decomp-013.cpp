#include "snoop.h"

// P2785 §implicit-decomposition [6.5]: pointer-to-data-member access.
// T elem = getT().*pmf; where pmf is constexpr.
// The resolved member is relocated out; other members are destroyed.

struct Base {
    snoop first;
    snoop arr[3];
    snoop second;
};

struct Aux { snoop aux; };

struct Intermediate : Base { snoop i; };

struct Derived : Intermediate, Aux { snoop d; };

Derived getDerived() {
    Derived p{snoop{"first"}, {snoop{"a"}, snoop{"b"}, snoop{"c"}}, snoop{"second"}, snoop{"Intermediate"}, snoop{"Aux"}, snoop{"Derived"}};
    std::cout << "---" << std::endl;
    return p;
}

int getRuntimeIndex() { return 1; }

int main(int, char**) {
    std::cout << "--- with constant index" << std::endl;
    {
        snoop s = getDerived().arr[1];
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "--- with runtime index" << std::endl;
    {
        snoop s = getDerived().arr[getRuntimeIndex()];
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// --- with constant index
// first() 0x1
// a() 0x2
// b() 0x3
// c() 0x4
// second() 0x5
// Intermediate() 0x6
// Aux() 0x7
// Derived() 0x8
// ---
// ~Derived() 0x8
// ~Aux() 0x7
// ~Intermediate() 0x6
// ~second() 0x5
// ~c() 0x4
// ~a() 0x2
// ~first() 0x1
// --- b alive ---
// ~b() 0x3
// --- with runtime index
// first() 0x9
// a() 0x10
// b() 0x11
// c() 0x12
// second() 0x13
// Intermediate() 0x14
// Aux() 0x15
// Derived() 0x16
// ---
// b(b reloc) 0x17 <- 0x11
// ~Derived() 0x16
// ~Aux() 0x15
// ~Intermediate() 0x14
// ~second() 0x13
// ~c() 0x12
// ~a() 0x10
// ~first() 0x9
// --- b alive ---
// ~b() 0x17
