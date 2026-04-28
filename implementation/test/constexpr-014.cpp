/// constexpr multiple sequential relocs and conditional reloc.

#include <iostream>

constexpr int test_sequential() {
    int a = 1, b = 2, c = 3;
    int x = reloc a;
    int y = reloc b;
    int z = reloc c;
    return x + y + z;
}

constexpr int test_conditional(bool cond) {
    int x = 10;
    if (cond) return reloc x;
    return x + 1;
}

static_assert(test_sequential() == 6, "");
static_assert(test_conditional(true) == 10, "");
static_assert(test_conditional(false) == 11, "");

int main()
{
    std::cout << "sequential: " << test_sequential() << std::endl;
    std::cout << "cond_true: " << test_conditional(true) << std::endl;
    std::cout << "cond_false: " << test_conditional(false) << std::endl;
    return 0;
}

////// BUILD SUCCESS
// sequential: 6
// cond_true: 10
// cond_false: 11
