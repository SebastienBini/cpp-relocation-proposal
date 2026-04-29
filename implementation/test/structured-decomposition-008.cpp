#include "snoop.h"

struct S {
    snoop u;
    snoop v;
    S() : u("U"), v("V") {}
    S const& operator reloc[]() {
        return *this;
    }
};

int main(int, char**)
{
    {
        S s;
        std::cout << "---" << std::endl;
        auto [a, b] = s;
        std::cout << "..." << std::endl;
        (void)reloc a;
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// U() 0x1
// V() 0x2
// ---
// U(U const&); 0x3 <- 0x1
// V(V const&); 0x4 <- 0x2
// ...
// ~U() 0x3
// ...
// ~V() 0x4
// ~V() 0x2
// ~U() 0x1
