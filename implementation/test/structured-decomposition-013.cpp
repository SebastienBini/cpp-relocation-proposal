// Templated `operator reloc[]` on a non-template class: the explicit object
// parameter is deduced (`this Self reloc self`), and the function template
// is instantiated separately for `S` and `S const`.  Each instance returns
// a different inner-aggregate type so we can observe which one was selected
// in the snoop output.

#include "snoop.h"
#include <type_traits>

struct S
{
    snoop u;
    snoop v;
    S() : u("U"), v("V") {}

    template <class Self>
    auto operator reloc[](this Self reloc self)
    {
        struct Mut { snoop a, b; };
        struct Cst { snoop a, b, c; };
        if constexpr (std::is_const_v<std::remove_reference_t<Self>>)
            return Cst{reloc self.u, reloc self.v, snoop{"W"}};
        else
            return Mut{reloc self.u, reloc self.v};
    }
};

int main(int, char**)
{
    {
        S s;
        std::cout << "--- non-const ---" << std::endl;
        auto [a, b] = reloc s;
        std::cout << "..." << std::endl;
    }
    std::cout << "===" << std::endl;
    {
        S const s;
        std::cout << "--- const ---" << std::endl;
        auto [a, b, c] = reloc s;
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD FAILURE
// structured-decomposition-013.cpp:40:14: error: type 'Mut' binds to 2 elements, but 3 names were provided
//    40 |         auto [a, b, c] = reloc s;
//       |              ^
// 1 error generated.
