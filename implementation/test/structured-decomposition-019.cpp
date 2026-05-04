#include "snoop.h"
#include <utility>

template <class T, std::size_t N>
class array
{
public:    
    auto operator reloc[](this array reloc self)
    {
        std::cout << "auto operator reloc[](this array reloc self) ---" << std::endl;
        auto [...ar_el] = reloc self._m;
        std::cout << "auto operator reloc[](this array reloc self) ---" << std::endl;
        return std::decomposition_pack{reloc ar_el...};
    }

private:
    T _m[N];
};

int main(int, char**)
{
    array<snoop, 3> arr;
    std::cout << "---" << std::endl;
    auto [a1, a2, a3] = reloc arr;
    std::cout << "---" << std::endl;
    reloc a1;
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// snoop() 0x2
// snoop() 0x3
// ---
// auto operator reloc[](this array reloc self) ---
// auto operator reloc[](this array reloc self) ---
// snoop(snoop reloc) 0x4 <- 0x1
// snoop(snoop reloc) 0x5 <- 0x2
// snoop(snoop reloc) 0x6 <- 0x3
// ---
// ~snoop() 0x4
// ---
// ~snoop() 0x6
// ~snoop() 0x5
