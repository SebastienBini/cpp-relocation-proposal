#include "snoop.h"


struct base
{
    int b{10};
    void print() { std::cout << "base print" << std::endl; }
    static void sprint() { std::cout << "base print static" << std::endl; }
};
struct D : base
{
    void print() { std::cout << "D print" << std::endl; }
    static void sprint() { std::cout << "D print static" << std::endl; }
    int d{20};
};

int main(int, char**)
{
    D reloc obj;
    std::cout << obj.d << std::endl;
    std::cout << obj.b << std::endl;
    std::cout << obj.base<base>.b << std::endl;
    obj.base<base>.print();
    constexpr auto pb = &D::b;
    constexpr auto pd = &D::d;
    std::cout << obj.*pb << std::endl;
    std::cout << obj.*pd << std::endl;
    obj.sprint();
    constexpr auto bprint = &base::print;
    constexpr auto dprint = &D::print;
    constexpr auto dsprint = &D::sprint;
    (obj.*bprint)();
    (obj.*dprint)();
    (*dsprint)();

    return 0;
}

////// BUILD SUCCESS
// 20
// 10
// 10
// base print
// 10
// 20
// D print static
// base print
// D print
// D print static
