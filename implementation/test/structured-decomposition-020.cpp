#include "snoop.h"

template <class T, std::size_t N>
class array
{
public:    
    void consume(this array reloc self)
    {
        std::cout << "consume ---" << std::endl;
        auto [...ar_el] = reloc self._m;
        std::cout << "consume ---" << std::endl;
        reloc ar_el...[1];
        std::cout << "consume ---" << std::endl;
    }

private:
    T _m[N];
};

int main(int, char**)
{
    std::cout << "array<snoop, 3>{}.consume() ---" << std::endl;
    array<snoop, 3>{}.consume();
    std::cout << "\nmake_arr().consume() ---" << std::endl;
    auto const make_arr = []() static { return array<snoop, 3>{}; };
    make_arr().consume();
    std::cout << "\n(reloc a).consume() ---" << std::endl;
    array<snoop, 3> a;
    (reloc a).consume();
    std::cout << "\nb.consume() ---" << std::endl;
    array<snoop, 3> b;
    b.consume();
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// array<snoop, 3>{}.consume() ---
// snoop() 0x1
// snoop() 0x2
// snoop() 0x3
// consume ---
// consume ---
// ~snoop() 0x2
// consume ---
// ~snoop() 0x3
// ~snoop() 0x1
//
// make_arr().consume() ---
// snoop() 0x4
// snoop() 0x5
// snoop() 0x6
// consume ---
// consume ---
// ~snoop() 0x5
// consume ---
// ~snoop() 0x6
// ~snoop() 0x4
//
// (reloc a).consume() ---
// snoop() 0x7
// snoop() 0x8
// snoop() 0x9
// consume ---
// consume ---
// ~snoop() 0x8
// consume ---
// ~snoop() 0x9
// ~snoop() 0x7
//
// b.consume() ---
// snoop() 0x10
// snoop() 0x11
// snoop() 0x12
// snoop(snoop const&); 0x13 <- 0x10
// snoop(snoop const&); 0x14 <- 0x11
// snoop(snoop const&); 0x15 <- 0x12
// consume ---
// consume ---
// ~snoop() 0x14
// consume ---
// ~snoop() 0x15
// ~snoop() 0x13
// ---
// ~snoop() 0x12
// ~snoop() 0x11
// ~snoop() 0x10
