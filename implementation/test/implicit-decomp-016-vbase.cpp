#include "snoop.h"

// P2785 §implicit-decomposition Phase 5: array element member access.
// T field = (reloc array_var)[index].field;
// The targeted element is decomposed (field relocated, other subobjects
// destroyed individually), and all other array elements are fully destroyed.

struct Base {
    snoop first{"first"};
    snoop arr[3]{snoop{"a"}, snoop{"b"}, snoop{"c"}};
    snoop second{"second"};
};

struct Aux { snoop aux{"aux"}; };

struct Intermediate : virtual Base { snoop i{"Intermediate"}; };

struct Derived : Intermediate, Aux { snoop d{"Derived"}; };

int getRuntimeIndex() { return 1; }

int main(int, char**) {
    {
        Derived d[3];
        std::cout << "---" << std::endl;
        snoop s = (reloc d)[1].first;
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
// first() 0x9
// a() 0x10
// b() 0x11
// c() 0x12
// second() 0x13
// Intermediate() 0x14
// aux() 0x15
// Derived() 0x16
// first() 0x17
// a() 0x18
// b() 0x19
// c() 0x20
// second() 0x21
// Intermediate() 0x22
// aux() 0x23
// Derived() 0x24
// ---
// first(first&&) 0x25 <- 0x9
// ~Derived() 0x24
// ~aux() 0x23
// ~Intermediate() 0x22
// ~second() 0x21
// ~c() 0x20
// ~b() 0x19
// ~a() 0x18
// ~first() 0x17
// ~Derived() 0x16
// ~aux() 0x15
// ~Intermediate() 0x14
// ~second() 0x13
// ~c() 0x12
// ~b() 0x11
// ~a() 0x10
// ~first() 0x9
// ~Derived() 0x8
// ~aux() 0x7
// ~Intermediate() 0x6
// ~second() 0x5
// ~c() 0x4
// ~b() 0x3
// ~a() 0x2
// ~first() 0x1
// --- first alive ---
// ~first() 0x25
