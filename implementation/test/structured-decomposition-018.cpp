// β rule (P2785 §"customized decomposition"): when every viable
// `operator reloc[]` candidate fails its `requires` clause, decomposition
// silently falls back to the data-members protocol.  The snoop output for
// `auto [a, b] = reloc s;` therefore shows two snoop relocations from the
// underlying members `u` and `v` — and *no* "op reloc[] picked" message.

#include "snoop.h"
#include <type_traits>

template <class T>
concept Tagged = requires { typename T::tag; };

struct S
{
    snoop u;
    snoop v;
    S() : u("U"), v("V") {}

    // Only viable if T is "Tagged"; for the plain S below the requires
    // clause fails, the candidate is not viable, and the decomposition
    // protocol falls through to the data-members protocol.
    template <class Self>
        requires Tagged<std::remove_reference_t<Self>>
    auto operator reloc[](this Self reloc self)
    {
        std::cout << "op reloc[] picked (should not happen)" << std::endl;
        struct R { snoop a, b; };
        return R{reloc self.u, reloc self.v};
    }
};

int main(int, char**)
{
    {
        S s;
        std::cout << "---" << std::endl;
        auto [a, b] = reloc s;
        std::cout << "..." << std::endl;
        reloc b;
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// U() 0x1
// V() 0x2
// ---
// ...
// ~V() 0x2
// ...
// ~U() 0x1
