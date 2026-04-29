#include "snoop.h"
#include "box.h"

struct Pair {
    snoop x;
    box<snoop> y;
    Pair() : x("X"), y("Y") {}
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
// ...
// ~Y() 0x2
// ...
// ~X() 0x1
