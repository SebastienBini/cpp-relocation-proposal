#include "snoop.h"
#include "box.h"

int main(int, char**)
{
    box<snoop> const b{"box<snoop>"};
    return 0;
}

////// BUILD SUCCESS
// box<snoop>() 0x1
// ~box<snoop>() 0x1
