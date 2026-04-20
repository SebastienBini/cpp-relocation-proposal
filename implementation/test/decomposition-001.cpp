#include "snoop.h"

struct S : public snoop
{
    S() : snoop("snoop") {}
    int a;
};

int main(int, char**)
{
    S a;
    auto reloc b = reloc a;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// ~snoop() 0x1
