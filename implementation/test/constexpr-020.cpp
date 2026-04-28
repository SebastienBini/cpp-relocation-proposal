/// consteval function: reloc ctor preference is not affected by
/// reloc ctor discardment (which only applies at runtime for ABI reasons).

#include <iostream>

struct S {
    int val;
    constexpr S(int v) : val(v) {}
    constexpr S(S reloc src) : val(src.val + 100) {}
    constexpr S(S&& src) : val(src.val + 1) {}
    constexpr ~S() {}
};

consteval int f(S s) {
    S t = reloc s;
    return t.val;
}

// consteval: reloc ctor always used (+100). 42+100=142.
static_assert(f(S(42)) == 142, "");
static_assert(f(S(0)) == 100, "");

int main()
{
    constexpr int r1 = f(S(42));
    constexpr int r2 = f(S(0));
    std::cout << "consteval_42: " << r1 << std::endl;
    std::cout << "consteval_0: " << r2 << std::endl;
    return 0;
}

////// BUILD SUCCESS
// consteval_42: 142
// consteval_0: 100
