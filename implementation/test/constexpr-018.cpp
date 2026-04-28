/// constexpr reloc in a loop: each iteration creates and consumes a local.
/// Verifies cleanup deactivation works across iterations.

#include <iostream>

constexpr int test() {
    int sum = 0;
    for (int i = 0; i < 5; ++i) {
        int x = i * 10;
        sum += reloc x;
    }
    return sum;
}

// 0 + 10 + 20 + 30 + 40 = 100
static_assert(test() == 100, "");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 100
// runtime: 100
