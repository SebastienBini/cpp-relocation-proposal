#include "snoop.h"

void bar(snoop&&) {std::cout << __func__ << std::endl;}

void foo(snoop s)
{
    std::cout << __func__ << " ---beg" << std::endl;
    bar(reloc s);
    std::cout << __func__ << " ---end" << std::endl;
}

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
// foo ---beg
// snoop(snoop&&) 0x2 <- 0x1
// bar
// ~snoop() 0x2
// foo ---end
// ~snoop() 0x1
// ---
