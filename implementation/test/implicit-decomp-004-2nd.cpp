#include "snoop.h"

// P2785 §implicit-decomposition [6.1]: derived-to-base cast with complex
// hierarchy.  C : Pre, B, Post where B : A.  Each class has two snoop members.
// Test mandatory relocation elision for all base combinations.

struct Pre  { snoop p1, p2; };
struct Post { snoop o1, o2; };
struct A    { snoop a1, a2; };
struct B : A { snoop b1, b2; };
struct C : Pre, B, Post { snoop c1, c2; };

C getC() {
    C c{snoop{"pre1"}, snoop{"pre2"},
             snoop{"a1"}, snoop{"a2"}, snoop{"b1"}, snoop{"b2"},
             snoop{"post1"}, snoop{"post2"},
             snoop{"c1"}, snoop{"c2"}};
    std::cout << "---" << std::endl;
    return c;
}

int main(int, char**) {
    // --- Cast to Pre (direct base, first) ---
    std::cout << "=== C -> Pre (prvalue) ===" << std::endl;
    {
        Pre p = getC();
        std::cout << "--- p alive ---" << std::endl;
    }
    std::cout << "=== C -> Pre (reloc) ===" << std::endl;
    {
        C c{getC()};
        Pre p = reloc c;
        std::cout << "--- p alive ---" << std::endl;
    }

    // --- Cast to Post (direct base, last) ---
    std::cout << "=== C -> Post (prvalue) ===" << std::endl;
    {
        Post p = getC();
        std::cout << "--- p alive ---" << std::endl;
    }
    std::cout << "=== C -> Post (reloc) ===" << std::endl;
    {
        C c{getC()};
        Post p = reloc c;
        std::cout << "--- p alive ---" << std::endl;
    }

    // --- Cast to B (direct base, middle, has own base A) ---
    std::cout << "=== C -> B (prvalue) ===" << std::endl;
    {
        B b = getC();
        std::cout << "--- b alive ---" << std::endl;
    }
    std::cout << "=== C -> B (reloc) ===" << std::endl;
    {
        C c{getC()};
        B b = reloc c;
        std::cout << "--- b alive ---" << std::endl;
    }

    // --- Cast to A (indirect base: C -> B -> A) ---
    // §implicit-decomposition: "Further implicit decompositions occur for each
    // ... cast to a base class" — chained decomposition through B.
    std::cout << "=== C -> A (prvalue) ===" << std::endl;
    {
        A a = getC();
        std::cout << "--- a alive ---" << std::endl;
    }
    std::cout << "=== C -> A (reloc) ===" << std::endl;
    {
        C c{getC()};
        A a = reloc c;
        std::cout << "--- a alive ---" << std::endl;
    }

    return 0;
}

////// BUILD SUCCESS
// === C -> Pre (prvalue) ===
// pre1() 0x1
// pre2() 0x2
// a1() 0x3
// a2() 0x4
// b1() 0x5
// b2() 0x6
// post1() 0x7
// post2() 0x8
// c1() 0x9
// c2() 0x10
// ---
// ~c2() 0x10
// ~c1() 0x9
// ~post2() 0x8
// ~post1() 0x7
// ~b2() 0x6
// ~b1() 0x5
// ~a2() 0x4
// ~a1() 0x3
// --- p alive ---
// ~pre2() 0x2
// ~pre1() 0x1
// === C -> Pre (reloc) ===
// pre1() 0x11
// pre2() 0x12
// a1() 0x13
// a2() 0x14
// b1() 0x15
// b2() 0x16
// post1() 0x17
// post2() 0x18
// c1() 0x19
// c2() 0x20
// ---
// ~c2() 0x20
// ~c1() 0x19
// ~post2() 0x18
// ~post1() 0x17
// ~b2() 0x16
// ~b1() 0x15
// ~a2() 0x14
// ~a1() 0x13
// --- p alive ---
// ~pre2() 0x12
// ~pre1() 0x11
// === C -> Post (prvalue) ===
// pre1() 0x21
// pre2() 0x22
// a1() 0x23
// a2() 0x24
// b1() 0x25
// b2() 0x26
// post1() 0x27
// post2() 0x28
// c1() 0x29
// c2() 0x30
// ---
// ~c2() 0x30
// ~c1() 0x29
// ~b2() 0x26
// ~b1() 0x25
// ~a2() 0x24
// ~a1() 0x23
// ~pre2() 0x22
// ~pre1() 0x21
// --- p alive ---
// ~post2() 0x28
// ~post1() 0x27
// === C -> Post (reloc) ===
// pre1() 0x31
// pre2() 0x32
// a1() 0x33
// a2() 0x34
// b1() 0x35
// b2() 0x36
// post1() 0x37
// post2() 0x38
// c1() 0x39
// c2() 0x40
// ---
// ~c2() 0x40
// ~c1() 0x39
// ~b2() 0x36
// ~b1() 0x35
// ~a2() 0x34
// ~a1() 0x33
// ~pre2() 0x32
// ~pre1() 0x31
// --- p alive ---
// ~post2() 0x38
// ~post1() 0x37
// === C -> B (prvalue) ===
// pre1() 0x41
// pre2() 0x42
// a1() 0x43
// a2() 0x44
// b1() 0x45
// b2() 0x46
// post1() 0x47
// post2() 0x48
// c1() 0x49
// c2() 0x50
// ---
// ~c2() 0x50
// ~c1() 0x49
// ~post2() 0x48
// ~post1() 0x47
// ~pre2() 0x42
// ~pre1() 0x41
// --- b alive ---
// ~b2() 0x46
// ~b1() 0x45
// ~a2() 0x44
// ~a1() 0x43
// === C -> B (reloc) ===
// pre1() 0x51
// pre2() 0x52
// a1() 0x53
// a2() 0x54
// b1() 0x55
// b2() 0x56
// post1() 0x57
// post2() 0x58
// c1() 0x59
// c2() 0x60
// ---
// ~c2() 0x60
// ~c1() 0x59
// ~post2() 0x58
// ~post1() 0x57
// ~pre2() 0x52
// ~pre1() 0x51
// --- b alive ---
// ~b2() 0x56
// ~b1() 0x55
// ~a2() 0x54
// ~a1() 0x53
// === C -> A (prvalue) ===
// pre1() 0x61
// pre2() 0x62
// a1() 0x63
// a2() 0x64
// b1() 0x65
// b2() 0x66
// post1() 0x67
// post2() 0x68
// c1() 0x69
// c2() 0x70
// ---
// ~c2() 0x70
// ~c1() 0x69
// ~post2() 0x68
// ~post1() 0x67
// ~b2() 0x66
// ~b1() 0x65
// ~pre2() 0x62
// ~pre1() 0x61
// --- a alive ---
// ~a2() 0x64
// ~a1() 0x63
// === C -> A (reloc) ===
// pre1() 0x71
// pre2() 0x72
// a1() 0x73
// a2() 0x74
// b1() 0x75
// b2() 0x76
// post1() 0x77
// post2() 0x78
// c1() 0x79
// c2() 0x80
// ---
// ~c2() 0x80
// ~c1() 0x79
// ~post2() 0x78
// ~post1() 0x77
// ~b2() 0x76
// ~b1() 0x75
// ~pre2() 0x72
// ~pre1() 0x71
// --- a alive ---
// ~a2() 0x74
// ~a1() 0x73
