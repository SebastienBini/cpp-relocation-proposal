#include "virtual-bases-relocatable-non-movable.h"

void decomp_d(D reloc d)
{
    std::cout << "decomp_d body" << std::endl;
}

int main(int, char**)
{
    std::cout << "main begin" << std::endl;
    D d;
    std::cout << "After D ctor" << std::endl;
    decomp_d(reloc d);
    std::cout << "main end" << std::endl;
}

////// BUILD SUCCESS
// main begin
// vb_m() 0x1
// VBase() 0x1
// b1_m() 0x2
// B1() 0x3
// b2_m() 0x4
// B2() 0x5
// c_m() 0x6
// C() 0x6
// d_m() 0x7
// D() 0x3
// After D ctor
// decomp_d body
// ~d_m() 0x7
// ~C() 0x6
// ~c_m() 0x6
// ~B2() 0x5
// ~b2_m() 0x4
// ~B1() 0x3
// ~b1_m() 0x2
// ~VBase() 0x1
// ~vb_m() 0x1
// main end
