#include "snoop.h"

struct A : public snoop { using snoop::snoop; };

int main(int, char**)
{
    A a{"A"};
    A b = reloc a;
    return 0;
}

////// BUILD SUCCESS
// A() 0x1
// ~A() 0x1
