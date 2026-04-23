#include <array>
#include "snoop.h"


int main(int, char**)
{
    std::array<snoop, 3> a = {snoop{"alpha"}, snoop{"beta"}, snoop{"gamma"}};
    std::cout << "a init-ed" << std::endl;
    std::array<snoop, 3> b = reloc a;
    std::cout << "b init-ed" << std::endl;
    std::array<snoop, 3> c = {snoop{"delta"}, snoop{"epsilon"}, snoop{"zeta"}};
    std::cout << "c init-ed" << std::endl;
    c = reloc b;
    std::cout << "c assigned" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// alpha() 0x1
// beta() 0x2
// gamma() 0x3
// a init-ed
// b init-ed
// delta() 0x4
// epsilon() 0x5
// zeta() 0x6
// c init-ed
// alpha& alpha::operator=(alpha reloc) 0x4 = 0x1
// beta& beta::operator=(beta reloc) 0x5 = 0x2
// gamma& gamma::operator=(gamma reloc) 0x6 = 0x3
// c assigned
// ~gamma() 0x6
// ~beta() 0x5
// ~alpha() 0x4
