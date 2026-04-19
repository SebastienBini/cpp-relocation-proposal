#include "snoop.h"
#include "box.h"
int main(int, char**)
{
    box<snoop> const b{"box<snoop>"};
    auto c = reloc b;
    std::cout << c.get() << std::endl;
    std::cout << "released " << (reloc c).release() << std::endl;
    return 0;
}

////// BUILD SUCCESS
// box<snoop>() 0x1
// 0x1
// released 0x1
