#include "virtual-bases.h"

void decomp_b1(B1 reloc b1)
{
    std::cout << "decomp_b1 beg" << std::endl;
    reloc b1.b1_m;
    std::cout << "decomp_b1 end" << std::endl;
}

void decomp_d(D reloc d)
{
    std::cout << "decomp_d beg" << std::endl;
    decomp_b1(reloc d.base<B1>);
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
// vb_m(vb_m&&) 0x8 <- 0x1
// VBase(VBase&&) 0x8 <- 0x1
// b1_m(b1_m reloc) 0x9 <- 0x2
// B1(B1 reloc) 0x10 <- 0x3
// decomp_b1 beg
// ~b1_m() 0x9
// decomp_b1 end
// ~VBase() 0x8
// ~vb_m() 0x8
// decomp_d end
// ~d_m() 0x7
// ~C() 0x6
// ~c_m() 0x6
// ~B2() 0x5
// ~b2_m() 0x4
// ~VBase() 0x1
// ~vb_m() 0x1
// main end
