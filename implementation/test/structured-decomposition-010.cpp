#include "snoop.h"
#include "box.h"


constexpr int run() {
    int const arr[3] = {12, 23, 45};
    auto const [a, b, c] = reloc arr;
    reloc b;
    return a;
}

static_assert(run() == 12);

int main(int, char**)
{
    std::cout << run() << std::endl;
    return 0;
}

////// BUILD SUCCESS
// 12
