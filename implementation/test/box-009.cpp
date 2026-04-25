#include "snoop.h"
#include "box.h"

int main(int, char**)
{
    box<snoop> b;
    auto getBox = [b2 = reloc b](this auto reloc self) { return self.b2; };
    auto b3 = (reloc getBox)();
    std::cout << "---" <<std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// ---
// ~snoop() 0x1
