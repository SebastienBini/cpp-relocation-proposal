#include "snoop.h"
#include "box.h"

template <int = 0>
void run() {
    box<snoop> arr[3] = {box<snoop>("a"), box<snoop>("b"), box<snoop>("c")};
    std::cout << "---" << std::endl;
    auto [...a] = reloc arr;
    std::cout << "..." << std::endl;
    reloc a...[1];
    std::cout << "..." << std::endl;
    auto d = reloc a...[0];
    std::cout << "..." << std::endl;
}

int main(int, char**)
{
    run();
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
