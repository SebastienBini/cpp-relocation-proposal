#include "virtual-bases-relocatable-non-movable.h"

template <class base>
base getBase(D reloc d)
{
    std::cout << "getBase beg" << std::endl;
    return d.base<base>;
    std::cout << "decomp_d end" << std::endl;
}

int main(int, char**)
{
    D d;
    std::cout << "D initialized" << std::endl;
    getBase<B1>(reloc d);
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
// getBase beg
// vb_m(vb_m&&) 0x8 <- 0x1
// VBase(VBase&&) 0x8 <- 0x1
// b1_m(b1_m reloc) 0x9 <- 0x2
// B1(B1 reloc) 0x10 <- 0x3
// ~d_m() 0x7
// ~C() 0x6
// ~c_m() 0x6
// ~B2() 0x5
// ~b2_m() 0x4
// ~VBase() 0x1
// ~vb_m() 0x1
// ~B1() 0x10
// ~b1_m() 0x9
// ~VBase() 0x8
// ~vb_m() 0x8
// main end
