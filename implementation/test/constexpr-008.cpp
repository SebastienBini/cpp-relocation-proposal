/// constexpr object decomposition: base access and member access.

#include <iostream>

struct B {
    int x;
    constexpr B(int v) : x(v) {}
};
struct D : B {
    int y;
    constexpr D(int a, int b) : B(a), y(b) {}
};

constexpr int test_single_base() {
    D reloc d(10, 20);
    return d.base<B>.x + d.y;
}

struct A { int a; constexpr A(int v) : a(v) {} };
struct C { int c; constexpr C(int v) : c(v) {} };
struct E : A, C {
    int e;
    constexpr E(int x, int y, int z) : A(x), C(y), e(z) {}
};

constexpr int test_multiple_bases() {
    E reloc obj(1, 2, 3);
    return obj.base<A>.a + obj.base<C>.c + obj.e;
}

constexpr int test_multiple_bases_2() {
    E reloc obj(1, 2, 3);
    return obj.a + obj.c + obj.e;
}

static_assert(test_single_base() == 30, "");
static_assert(test_multiple_bases() == 6, "");
static_assert(test_multiple_bases_2() == 6, "");

int main()
{
    std::cout << "single_base: " << test_single_base() << std::endl;
    std::cout << "multiple_bases: " << test_multiple_bases() << std::endl;
    return 0;
}

////// BUILD SUCCESS
// single_base: 30
// multiple_bases: 6
