#include "virtual-bases-relocatable-non-movable.h"


void decomp_d(D reloc d)
{
    std::cout << "decomp_d begin" << std::endl;
    auto b = reloc d.base<VBase>;
    std::cout << "decomp_d end" << std::endl;
}

int main(int, char**)
{
    std::cout << "main begin" << std::endl;
    D d;
    std::cout << "After D ctor" << std::endl;
    decomp_d(reloc d);
    std::cout << "main end" << std::endl;
}

////// BUILD FAILURE
// virtual-bases-002rnm.cpp:7:14: error: cannot relocate virtual base 'VBase' of a decomposed object
//     7 |     auto b = reloc d.base<VBase>;
//       |              ^
// 1 error generated.
