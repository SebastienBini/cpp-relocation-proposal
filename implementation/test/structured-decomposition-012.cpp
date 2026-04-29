#include "snoop.h"
#include "box.h"


struct S
{
    int a;
    int b;
};

constexpr int run() {
    auto const [a, b] = S{12, 23};
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
