#include "snoop.h"
#include "box.h"


int main(int, char**)
{
    {
        box<snoop> arr[3] = {box<snoop>("a"), box<snoop>("b"), box<snoop>("c")};
        std::cout << "---" << std::endl;
        auto [a, b, c] = reloc arr;
        std::cout << "..." << std::endl;
        reloc b;
        std::cout << "..." << std::endl;
        auto d = reloc a;
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// a() 0x1
// b() 0x2
// c() 0x3
// ---
// ...
// ~b() 0x2
// ...
// ...
// ~a() 0x1
// ~c() 0x3
