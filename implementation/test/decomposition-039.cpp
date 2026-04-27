#include <iostream>

struct S {};

void foo(S reloc) {}

template <void (*F)(S reloc)>
void tpl() {}

int main()
{
    void (*a)(S reloc) = &foo;
    static_assert(std::is_same_v<decltype(&foo), void (*)(S reloc)>);
    tpl<&foo>();
    return 0;
}

////// BUILD FAILURE
// decomposition-039.cpp:7:23: error: 'reloc' cannot appear in a function type; it is only valid in a function declaration or definition
//     7 | template <void (*F)(S reloc)>
//       |                       ^
// decomposition-039.cpp:12:17: error: 'reloc' cannot appear in a function type; it is only valid in a function declaration or definition
//    12 |     void (*a)(S reloc) = &foo;
//       |                 ^
// decomposition-039.cpp:13:61: error: 'reloc' cannot appear in a function type; it is only valid in a function declaration or definition
//    13 |     static_assert(std::is_same_v<decltype(&foo), void (*)(S reloc)>);
//       |                                                             ^
// 3 errors generated.
