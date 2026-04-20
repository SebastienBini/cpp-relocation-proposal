#include "snoop.h"
#include "box.h"

void foo(box<snoop> b)
{
    std::cout << "snoop must be alive" << std::endl;
    reloc b;
    std::cout << "snoop must be destroyed" << std::endl;
}

int main(int, char**)
{
    box<snoop> const b{"box<snoop>"};
    foo(reloc b);
    return 0;
}

////// BUILD SUCCESS
// box<snoop>() 0x1
// snoop must be alive
// ~box<snoop>() 0x1
// snoop must be destroyed
