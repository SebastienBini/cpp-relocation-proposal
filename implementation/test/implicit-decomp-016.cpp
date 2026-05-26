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

struct Intermediate : Base { snoop i{"Intermediate"}; };

struct Derived : Intermediate, Aux { snoop d{"Derived"}; };

int getRuntimeIndex() { return 1; }

int main(int, char**) {
    std::cout << "--- with constant index 1/2" << std::endl;
    {
        Derived d[3];
        std::cout << "---" << std::endl;
        snoop s = (reloc d)[1].first;
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "--- with constant index 2/2" << std::endl;
    {
        Derived d[3];
        std::cout << "---" << std::endl;
        snoop s = (reloc d)[1].arr[0];
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "--- with runtime index 1/2" << std::endl;
    {
        Derived d[3];
        std::cout << "---" << std::endl;
        snoop s = (reloc d)[getRuntimeIndex()].first;
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "--- with runtime index 2/2" << std::endl;
    {
        Derived d[3];
        std::cout << "---" << std::endl;
        snoop s = (reloc d)[getRuntimeIndex()].arr[getRuntimeIndex()];
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// --- with constant index 1/2
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
// ~Derived() 0x8
// ~aux() 0x7
// ~Intermediate() 0x6
// ~second() 0x5
// ~c() 0x4
// ~b() 0x3
// ~a() 0x2
// ~first() 0x1
// --- first alive ---
// ~first() 0x9
// --- with constant index 2/2
// first() 0x25
// a() 0x26
// b() 0x27
// c() 0x28
// second() 0x29
// Intermediate() 0x30
// aux() 0x31
// Derived() 0x32
// first() 0x33
// a() 0x34
// b() 0x35
// c() 0x36
// second() 0x37
// Intermediate() 0x38
// aux() 0x39
// Derived() 0x40
// first() 0x41
// a() 0x42
// b() 0x43
// c() 0x44
// second() 0x45
// Intermediate() 0x46
// aux() 0x47
// Derived() 0x48
// ---
// ~Derived() 0x48
// ~aux() 0x47
// ~Intermediate() 0x46
// ~second() 0x45
// ~c() 0x44
// ~b() 0x43
// ~a() 0x42
// ~first() 0x41
// ~Derived() 0x40
// ~aux() 0x39
// ~Intermediate() 0x38
// ~second() 0x37
// ~c() 0x36
// ~b() 0x35
// ~first() 0x33
// ~Derived() 0x32
// ~aux() 0x31
// ~Intermediate() 0x30
// ~second() 0x29
// ~c() 0x28
// ~b() 0x27
// ~a() 0x26
// ~first() 0x25
// --- a alive ---
// ~a() 0x34
// --- with runtime index 1/2
// first() 0x49
// a() 0x50
// b() 0x51
// c() 0x52
// second() 0x53
// Intermediate() 0x54
// aux() 0x55
// Derived() 0x56
// first() 0x57
// a() 0x58
// b() 0x59
// c() 0x60
// second() 0x61
// Intermediate() 0x62
// aux() 0x63
// Derived() 0x64
// first() 0x65
// a() 0x66
// b() 0x67
// c() 0x68
// second() 0x69
// Intermediate() 0x70
// aux() 0x71
// Derived() 0x72
// ---
// first(first reloc) 0x73 <- 0x57
// ~Derived() 0x72
// ~aux() 0x71
// ~Intermediate() 0x70
// ~second() 0x69
// ~c() 0x68
// ~b() 0x67
// ~a() 0x66
// ~first() 0x65
// ~Derived() 0x64
// ~aux() 0x63
// ~Intermediate() 0x62
// ~second() 0x61
// ~c() 0x60
// ~b() 0x59
// ~a() 0x58
// ~Derived() 0x56
// ~aux() 0x55
// ~Intermediate() 0x54
// ~second() 0x53
// ~c() 0x52
// ~b() 0x51
// ~a() 0x50
// ~first() 0x49
// --- first alive ---
// ~first() 0x73
// --- with runtime index 2/2
// first() 0x74
// a() 0x75
// b() 0x76
// c() 0x77
// second() 0x78
// Intermediate() 0x79
// aux() 0x80
// Derived() 0x81
// first() 0x82
// a() 0x83
// b() 0x84
// c() 0x85
// second() 0x86
// Intermediate() 0x87
// aux() 0x88
// Derived() 0x89
// first() 0x90
// a() 0x91
// b() 0x92
// c() 0x93
// second() 0x94
// Intermediate() 0x95
// aux() 0x96
// Derived() 0x97
// ---
// b(b reloc) 0x98 <- 0x84
// ~Derived() 0x97
// ~aux() 0x96
// ~Intermediate() 0x95
// ~second() 0x94
// ~c() 0x93
// ~b() 0x92
// ~a() 0x91
// ~first() 0x90
// ~Derived() 0x89
// ~aux() 0x88
// ~Intermediate() 0x87
// ~second() 0x86
// ~c() 0x85
// ~a() 0x83
// ~first() 0x82
// ~Derived() 0x81
// ~aux() 0x80
// ~Intermediate() 0x79
// ~second() 0x78
// ~c() 0x77
// ~b() 0x76
// ~a() 0x75
// ~first() 0x74
// --- b alive ---
// ~b() 0x98
