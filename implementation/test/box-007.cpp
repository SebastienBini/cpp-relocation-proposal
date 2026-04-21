#include "snoop.h"
#include "box.h"

int main(int, char**)
{
    box<snoop> const b{"box<snoop>"};
    std::cout << "after b init" << std::endl;
    box<snoop> b2{"box<snoop>2"};
    std::cout << "after b2 init" << std::endl;
    b2 = reloc b;
    std::cout << "after b2 assignment" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// box<snoop>() 0x1
// after b init
// box<snoop>2() 0x2
// after b2 init
// ~box<snoop>2() 0x2
// after b2 assignment
// ~box<snoop>() 0x1
