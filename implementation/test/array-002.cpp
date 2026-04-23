#include <array>
#include <memory>
#include "snoop.h"


int main(int, char**)
{
    std::array<std::unique_ptr<snoop>, 3> a;
    for (auto& p : a) p.reset(new snoop);
    std::cout << "a init-ed" << std::endl;
    std::array<std::unique_ptr<snoop>, 3> b = reloc a;
    std::cout << "b init-ed" << std::endl;
    std::array<std::unique_ptr<snoop>, 3> c;
    for (auto& p : c) p.reset(new snoop);
    std::cout << "c init-ed" << std::endl;
    c = reloc b;
    std::cout << "c assigned" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// snoop() 0x2
// snoop() 0x3
// a init-ed
// b init-ed
// snoop() 0x4
// snoop() 0x5
// snoop() 0x6
// c init-ed
// ~snoop() 0x4
// ~snoop() 0x5
// ~snoop() 0x6
// c assigned
// ~snoop() 0x3
// ~snoop() 0x2
// ~snoop() 0x1
