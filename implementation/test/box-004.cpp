#include "snoop.h"
#include "box.h"

int main(int, char**)
{
    box<snoop> const b{"box<snoop>"};
    std::cout << "snoop must be alive" << std::endl;
    reloc b;
    std::cout << "snoop must be destroyed" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// box<snoop>() 0x1
// snoop must be alive
// ~box<snoop>() 0x1
// snoop must be destroyed
