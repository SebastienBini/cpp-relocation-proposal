#include "snoop.h"
#include "intermediate-bases.h"

#include <array>
#include <string_view>

struct Base {
    snoop first{"first"};
    snoop arr[3]{snoop{"a"}, snoop{"b"}, snoop{"c"}};
    snoop second{"second"};
};

struct Aux { snoop aux{"aux"}; };

struct Derived : Intermediate<5, Base>, Aux { snoop d{"Derived"}; };

Derived getDerived() {
    Derived p;
    std::cout << "---" << std::endl;
    return p;
}

int getRuntimeIndex() { return 1; }

int main(int, char**) {
    std::cout << "--- second" << std::endl;
    {
        snoop s = getDerived().second;
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
    std::cout << "--- aux" << std::endl;
    {
        snoop s = getDerived().aux;
        std::cout << "--- " << s.name << " alive ---" << std::endl;
    }
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
// --- second
// first() 0x1
// a() 0x2
// b() 0x3
// c() 0x4
// second() 0x5
// Intermediate() 0x6
// 2() 0x7
// 3() 0x8
// 4() 0x9
// 5() 0x10
// aux() 0x11
// Derived() 0x12
// ---
// ~Derived() 0x12
// ~aux() 0x11
// ~5() 0x10
// ~4() 0x9
// ~3() 0x8
// ~2() 0x7
// ~Intermediate() 0x6
// ~c() 0x4
// ~b() 0x3
// ~a() 0x2
// ~first() 0x1
// --- second alive ---
// ~second() 0x5
// --- aux
// first() 0x13
// a() 0x14
// b() 0x15
// c() 0x16
// second() 0x17
// Intermediate() 0x18
// 2() 0x19
// 3() 0x20
// 4() 0x21
// 5() 0x22
// aux() 0x23
// Derived() 0x24
// ---
// ~Derived() 0x24
// ~5() 0x22
// ~4() 0x21
// ~3() 0x20
// ~2() 0x19
// ~Intermediate() 0x18
// ~second() 0x17
// ~c() 0x16
// ~b() 0x15
// ~a() 0x14
// ~first() 0x13
// --- aux alive ---
// ~aux() 0x23
// --- with constant index
// first() 0x25
// a() 0x26
// b() 0x27
// c() 0x28
// second() 0x29
// Intermediate() 0x30
// 2() 0x31
// 3() 0x32
// 4() 0x33
// 5() 0x34
// aux() 0x35
// Derived() 0x36
// ---
// ~Derived() 0x36
// ~aux() 0x35
// ~5() 0x34
// ~4() 0x33
// ~3() 0x32
// ~2() 0x31
// ~Intermediate() 0x30
// ~second() 0x29
// ~c() 0x28
// ~a() 0x26
// ~first() 0x25
// --- b alive ---
// ~b() 0x27
// --- with runtime index
// first() 0x37
// a() 0x38
// b() 0x39
// c() 0x40
// second() 0x41
// Intermediate() 0x42
// 2() 0x43
// 3() 0x44
// 4() 0x45
// 5() 0x46
// aux() 0x47
// Derived() 0x48
// ---
// b(b reloc) 0x49 <- 0x39
// ~Derived() 0x48
// ~aux() 0x47
// ~5() 0x46
// ~4() 0x45
// ~3() 0x44
// ~2() 0x43
// ~Intermediate() 0x42
// ~second() 0x41
// ~c() 0x40
// ~a() 0x38
// ~first() 0x37
// --- b alive ---
// ~b() 0x49
