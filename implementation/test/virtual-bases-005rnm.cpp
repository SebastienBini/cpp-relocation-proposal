#include "virtual-bases-relocatable-non-movable.h"


void decomp_d(D reloc d)
{
    std::cout << "decomp_d beg" << std::endl;
    reloc d.base<B1>;
    std::cout << "after: reloc d.base<B1>;" << std::endl;
    auto b2 = reloc d.base<B2>;
    std::cout << "decomp_d end" << std::endl;
}

int main(int, char**)
{
    D d;
    std::cout << "D initialized" << std::endl;
    decomp_d(reloc d);
    std::cout << "main end" << std::endl;
}

////// BUILD SUCCESS
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
// D initialized
// decomp_d beg
// ~B1() 0x3
// ~b1_m() 0x2
// after: reloc d.base<B1>;
// vb_m(vb_m&&) 0x8 <- 0x1
// VBase(VBase&&) 0x8 <- 0x1
// b2_m(b2_m reloc) 0x9 <- 0x4
// B2(B2 reloc) 0x10 <- 0x5
// decomp_d end
// ~B2() 0x10
// ~b2_m() 0x9
// ~VBase() 0x8
// ~vb_m() 0x8
// ~VBase() 0x1
// ~vb_m() 0x1
// ~C() 0x6
// ~c_m() 0x6
// ~d_m() 0x7
// main end
