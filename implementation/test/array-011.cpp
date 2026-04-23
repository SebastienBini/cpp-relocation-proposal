#include <array>
#include "snoop.h"
#include "box.h"


std::array<box<snoop>, 3> fwd_array(std::array<box<snoop>, 3> reloc a)
{
    return a;
}


int main(int, char**)
{
    std::array<box<snoop>, 3> a;
    std::cout << "a init-ed" << std::endl;
    std::array<box<snoop>, 3> b = fwd_array(reloc a);
    std::cout << "b init-ed" << std::endl;
    std::array<box<snoop>, 3> c;
    std::cout << "c init-ed" << std::endl;
    c = fwd_array(reloc b);
    std::cout << "c assigned" << std::endl;
    return 0;
}

////// BUILD FAILURE
// array-011.cpp:8:12: error: decomposed object 'a' does not yield a value; access individual members ('obj.member') or base subobjects ('obj.base<B>') instead
//     8 |     return a;
//       |            ^
// 1 error generated.
