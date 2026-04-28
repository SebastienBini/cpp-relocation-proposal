/// constexpr reloc with trivial defaulted reloc ctor.
/// Verifies the defaulted reloc ctor performs a bitwise copy in constexpr.

#include <iostream>

struct S {
    int a;
    double b;
    constexpr S(int x, double y) : a(x), b(y) {}
    constexpr S(S reloc) = default;
};

constexpr bool test() {
    S s(42, 3.14);
    S t = reloc s;
    return t.a == 42 && t.b == 3.14;
}

static_assert(test(), "");

int main()
{
    constexpr auto result = test();
    std::cout << "trivial_defaulted_reloc: " << (result ? "OK" : "FAIL") << std::endl;

    // Also test at runtime
    S s(99, 2.72);
    S t = reloc s;
    std::cout << "runtime: a=" << t.a << " b=" << t.b << std::endl;
    return 0;
}

////// BUILD SUCCESS
// trivial_defaulted_reloc: OK
// runtime: a=99 b=2.72
