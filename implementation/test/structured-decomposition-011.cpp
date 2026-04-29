#include "snoop.h"
#include "box.h"


struct S
{
    constexpr S(int a, int b) : a{a}, b{b} {}
    constexpr auto operator reloc[](this S reloc s)
    {
        struct T { int a, b; };
        return T{reloc s.a, reloc s.b};
    }

    int a;
private:
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
