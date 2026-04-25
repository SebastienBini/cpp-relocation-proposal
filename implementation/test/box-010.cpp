#include "snoop.h"
#include "box.h"

struct S { box<snoop> b; };

int main(int, char**)
{
    box<snoop> b;
    S s{reloc b};
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// ---
// ~snoop() 0x1
