#include "virtual-bases.h"


void decomp_d(D reloc d)
{
    auto b = reloc d.base<VBase>;
}

int main(int, char**)
{
    D d;
    decomp_d(reloc d);
}

////// BUILD FAILURE
// virtual-bases-002.cpp:6:14: error: cannot relocate virtual base 'VBase' of a decomposed object
//     6 |     auto b = reloc d.base<VBase>;
//       |              ^
// 1 error generated.
