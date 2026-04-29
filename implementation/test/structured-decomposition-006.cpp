#include "snoop.h"
#include "box.h"
#include <utility>

struct Pair {
    snoop x;
    box<snoop> y;
    Pair() : x("X"), y("Y") {}
    Pair(snoop x, box<snoop> y) : x(std::move(x)), y(reloc y) {}

    auto operator reloc[](this Pair reloc s)
    {
        std::cout << "operator reloc[]" << std::endl;
        return Pair{reloc s.x, reloc s.y};
    }
};

int main(int, char**)
{
    {
        Pair p;
        std::cout << "---" << std::endl;
        auto [a, b] = reloc p;
        std::cout << "..." << std::endl;
        reloc b;
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// X() 0x1
// Y() 0x2
// ---
// operator reloc[]
// X(X&&) 0x3 <- 0x1
// ~X() 0x1
// ...
// ~Y() 0x2
// ...
// ~X() 0x3
