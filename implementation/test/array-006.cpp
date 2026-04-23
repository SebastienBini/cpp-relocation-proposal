////// BUILD SUCCESS
#include <array>
#include "box.h"
#include "snoop.h"


int main(int, char**)
{
    std::array<std::array<box<snoop>, 3>, 2> a;
    std::cout << "a init-ed" << std::endl;
    std::array<std::array<box<snoop>, 3>, 2> b = reloc a;
    std::cout << "b init-ed" << std::endl;
    std::array<std::array<box<snoop>, 3>, 2> c;
    std::cout << "c init-ed" << std::endl;
    c = reloc b;
    std::cout << "c assigned" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// snoop() 0x2
// snoop() 0x3
// snoop() 0x4
// snoop() 0x5
// snoop() 0x6
// a init-ed
// b init-ed
// snoop() 0x7
// snoop() 0x8
// snoop() 0x9
// snoop() 0x10
// snoop() 0x11
// snoop() 0x12
// c init-ed
// ~snoop() 0x7
// ~snoop() 0x8
// ~snoop() 0x9
// ~snoop() 0x10
// ~snoop() 0x11
// ~snoop() 0x12
// c assigned
// ~snoop() 0x6
// ~snoop() 0x5
// ~snoop() 0x4
// ~snoop() 0x3
// ~snoop() 0x2
// ~snoop() 0x1
