#include <array>
#include "box.h"
#include "snoop.h"


int main(int, char**)
{
    std::array<box<snoop>, 10> a;
    std::array<box<snoop>, 10> b = reloc a;
    std::array<box<snoop>, 10> c;
    c = reloc b;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// snoop() 0x2
// snoop() 0x3
// snoop() 0x4
// snoop() 0x5
// snoop() 0x6
// snoop() 0x7
// snoop() 0x8
// snoop() 0x9
// snoop() 0x10
// snoop() 0x11
// snoop() 0x12
// snoop() 0x13
// snoop() 0x14
// snoop() 0x15
// snoop() 0x16
// snoop() 0x17
// snoop() 0x18
// snoop() 0x19
// snoop() 0x20
// ~snoop() 0x11
// ~snoop() 0x12
// ~snoop() 0x13
// ~snoop() 0x14
// ~snoop() 0x15
// ~snoop() 0x16
// ~snoop() 0x17
// ~snoop() 0x18
// ~snoop() 0x19
// ~snoop() 0x20
// ~snoop() 0x10
// ~snoop() 0x9
// ~snoop() 0x8
// ~snoop() 0x7
// ~snoop() 0x6
// ~snoop() 0x5
// ~snoop() 0x4
// ~snoop() 0x3
// ~snoop() 0x2
// ~snoop() 0x1
