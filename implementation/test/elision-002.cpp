#include "snoop.h"

void foo(snoop&&) {std::cout << __func__ << std::endl;}

int main()
{
    snoop s;
    std::cout << "---" << std::endl;
    foo(reloc s);
    std::cout << "---" << std::endl;
}

////// BUILD SUCCESS
// snoop() 0x1
// ---
// foo
// ~snoop() 0x1
// ---
