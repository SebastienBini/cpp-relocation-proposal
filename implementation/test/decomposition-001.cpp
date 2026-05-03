#include "snoop.h"

struct B : public snoop
{
    static constexpr std::string_view kClassName{"B"};
    snoop bx;
    snoop by;
    B() : snoop("B_Base"), bx{"bx"} , by{"by"} 
    { std::cout << kClassName << "() " << this << std::endl; }
    ~B() 
    { std::cout << '~' << kClassName << "() " << this << std::endl; }
};

struct D : public B, public snoop
{
    static constexpr std::string_view kClassName{"D"};
    snoop dx;
    snoop dy;
    D() : snoop("D_Base"), dx{"dx"} , dy{"dy"}
    { std::cout << kClassName << "() " << this << std::endl; }
    ~D() 
    { std::cout << '~' << kClassName << "() " << this << std::endl; }
    
    static void test() { D reloc a; }
};

int main(int, char**)
{
    std::cout << "---" << std::endl;
    {
        D a;
    }
    std::cout << "---" << std::endl;
    {
        D::test();
    }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---
// B_Base() 0x1
// bx() 0x2
// by() 0x3
// B() 0x1
// D_Base() 0x4
// dx() 0x5
// dy() 0x6
// D() 0x1
// ~D() 0x1
// ~dy() 0x6
// ~dx() 0x5
// ~D_Base() 0x4
// ~B() 0x1
// ~by() 0x3
// ~bx() 0x2
// ~B_Base() 0x1
// ---
// B_Base() 0x7
// bx() 0x8
// by() 0x9
// B() 0x7
// D_Base() 0x10
// dx() 0x11
// dy() 0x12
// D() 0x7
// ~dy() 0x12
// ~dx() 0x11
// ~D_Base() 0x10
// ~B() 0x7
// ~by() 0x9
// ~bx() 0x8
// ~B_Base() 0x7
// ---
