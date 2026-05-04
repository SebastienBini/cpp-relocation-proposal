/// Same as virtual-bases-005 but decomp_d is called through a
/// runtime function pointer instead of directly.

#include "virtual-bases.h"


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
    void (*fp)(D) = &decomp_d;
    fp(reloc d);
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
// vb_m(vb_m&&) 0x8 <- 0x1
// VBase(VBase&&) 0x8 <- 0x1
// b1_m(b1_m&&) 0x9 <- 0x2
// B1(B1&&) 0x10 <- 0x3
// b2_m(b2_m&&) 0x11 <- 0x4
// B2(B2&&) 0x12 <- 0x5
// c_m(c_m&&) 0x13 <- 0x6
// C(C&&) 0x13 <- 0x6
// d_m(d_m&&) 0x14 <- 0x7
// D(D&&) 0x10 <- 0x3
// decomp_d beg
// ~B1() 0x10
// ~b1_m() 0x9
// after: reloc d.base<B1>;
// vb_m(vb_m&&) 0x15 <- 0x8
// VBase(VBase&&) 0x15 <- 0x8
// b2_m(b2_m reloc) 0x16 <- 0x11
// B2(B2 reloc) 0x17 <- 0x12
// decomp_d end
// ~B2() 0x17
// ~b2_m() 0x16
// ~VBase() 0x15
// ~vb_m() 0x15
// ~d_m() 0x14
// ~C() 0x13
// ~c_m() 0x13
// ~VBase() 0x8
// ~vb_m() 0x8
// ~D() 0x3
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
