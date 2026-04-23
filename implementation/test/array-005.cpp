#include <array>
#include "box.h"
#include "snoop.h"


struct bundle {
    int id;
    std::array<box<snoop>, 3> items;
    double weight;
};

int main(int, char**)
{
    bundle a;
    a.id = 1;
    a.weight = 2.5;
    std::cout << "a init-ed" << std::endl;
    bundle b = reloc a;
    std::cout << "b init-ed" << std::endl;
    bundle c;
    c.id = 99;
    c.weight = 9.9;
    std::cout << "c init-ed" << std::endl;
    c = reloc b;
    std::cout << "c assigned" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// snoop() 0x2
// snoop() 0x3
// a init-ed
// b init-ed
// snoop() 0x4
// snoop() 0x5
// snoop() 0x6
// c init-ed
// ~snoop() 0x4
// ~snoop() 0x5
// ~snoop() 0x6
// c assigned
// ~snoop() 0x3
// ~snoop() 0x2
// ~snoop() 0x1
