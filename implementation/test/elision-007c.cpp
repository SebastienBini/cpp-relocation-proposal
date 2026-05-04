#include "snoop.h"
#include <utility>

struct S {
    snoop d;
    snoop e;
    S() : d{"d"}, e{"e"} {}
};

void test_member_elision(S const reloc self) {
    // Mandatory elision: a occupies the same storage as self.d
    snoop a = reloc self.d;
    std::cout << "a is at " << &a << std::endl;
    // a should have the same address as the original self.d
    // With elision, no reloc ctor is called for 'a'.
    std::cout << "---" << std::endl;
    reloc a;
    std::cout << "---" << std::endl;
    // self.e is destroyed at scope exit
}

int main(int, char**)
{
    S const s;
    std::cout << "s.d is at " << &s.d << std::endl;
    std::cout << "---" << std::endl;
    test_member_elision(reloc s);
    return 0;
}

////// BUILD SUCCESS
// d() 0x1
// e() 0x2
// s.d is at 0x1
// ---
// a is at 0x1
// ---
// ~d() 0x1
// ---
// ~e() 0x2
