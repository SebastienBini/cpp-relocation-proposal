#include "snoop.h"
#include "box.h"

void sink(box<snoop> b)
{
    std::cout << b.get() << std::endl;
    std::cout << "released" << std::endl;
    delete (reloc b).release();
}

int main(int, char**)
{
    box<snoop> const b{"box<snoop>"};
    auto c = reloc b;
    sink(reloc c);
    return 0;
}

////// BUILD SUCCESS
// box<snoop>() 0x1
// 0x1
// released
// ~box<snoop>() 0x1
