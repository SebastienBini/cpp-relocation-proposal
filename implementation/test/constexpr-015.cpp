/// constexpr chained reloc: reloc local → pass to function → reloc param.

#include <iostream>

struct S {
    int val;
    constexpr S(int v) : val(v) {}
    constexpr S(S reloc) = default;
};

constexpr S identity(S s) { return reloc s; }

constexpr int test() {
    S a(77);
    S b = identity(reloc a);
    return b.val;
}

static_assert(test() == 77, "");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 77
// runtime: 77
