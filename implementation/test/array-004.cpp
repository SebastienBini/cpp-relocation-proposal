#include <array>
#include "box.h"
#include "snoop.h"


std::array<box<snoop>, 3> make_array()
{
    std::array<box<snoop>, 3> local;
    std::cout << "local init-ed" << std::endl;
    return local;
}

int main(int, char**)
{
    std::array<box<snoop>, 3> a = make_array();
    std::cout << "a received" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// snoop() 0x2
// snoop() 0x3
// local init-ed
// a received
// ~snoop() 0x3
// ~snoop() 0x2
// ~snoop() 0x1
