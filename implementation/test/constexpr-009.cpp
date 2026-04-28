/// constexpr reloc of decomposed base subobject.

#include <iostream>

struct B {
    int x;
    constexpr B(int v) : x(v) {}
    constexpr B(B reloc) = default;
};
struct D : B {
    int y;
    constexpr D(int a, int b) : B(a), y(b) {}
};

constexpr int test() {
    D reloc d(10, 20);
    B b = reloc d.base<B>;
    return b.x;
}

static_assert(test() == 10, "");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 10
// runtime: 10
