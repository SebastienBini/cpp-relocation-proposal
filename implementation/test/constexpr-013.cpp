/// constexpr defaulted reloc ctor with base class inheritance.

#include <iostream>

struct B {
    int x;
    constexpr B(int v) : x(v) {}
    constexpr B(B reloc) = default;
};

struct D : B {
    int y;
    constexpr D(int a, int b) : B(a), y(b) {}
    constexpr D(D reloc) = default;
};

constexpr int test() {
    D d(1, 2);
    D e = reloc d;
    return e.x + e.y;
}

static_assert(test() == 3, "");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 3
// runtime: 3
