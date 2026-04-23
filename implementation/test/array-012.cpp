#include <memory>
#include <array>
#include "snoop.h"
#include "box.h"


void init(std::array<std::unique_ptr<snoop>, 3>& arr)
{
    for (auto& p : arr) p.reset(new snoop);
}

std::array<std::unique_ptr<snoop>, 3> fwd_array(std::array<std::unique_ptr<snoop>, 3> a)
{
    return a;
}


int main(int, char**)
{
    std::array<std::unique_ptr<snoop>, 3> a; init(a);
    std::cout << "a init-ed" << std::endl;
    std::array<std::unique_ptr<snoop>, 3> b = fwd_array(reloc a);
    std::cout << "b init-ed" << std::endl;
    std::array<std::unique_ptr<snoop>, 3> c; init(c);
    std::cout << "c init-ed" << std::endl;
    c = fwd_array(reloc b);
    std::cout << "c assigned" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// snoop() 0x2
// snoop() 0x3
// a init-ed
// b init-ed
// snoop() 0x4
// snoop() 0x5
// snoop() 0x6
// c init-ed
// ~snoop() 0x4
// ~snoop() 0x5
// ~snoop() 0x6
// c assigned
// ~snoop() 0x3
// ~snoop() 0x2
// ~snoop() 0x1
