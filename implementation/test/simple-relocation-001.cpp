#include "snoop.h"

struct A : public snoop { using snoop::snoop; };

int main(int, char**)
{
    std::cout << "main begin" << std::endl;
    A a{"A"};
    std::cout << "after A constructor" << std::endl;
    A b = reloc a;
    std::cout << "main end" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// main begin
// A() 0x1
// after A constructor
// main end
// ~A() 0x1
