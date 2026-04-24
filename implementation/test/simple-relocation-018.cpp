/// Reloc-assign is implicitly deleted when the type has a const non-class
/// data member (same rule as for move-assign and copy-assign).
/// 'a = reloc b' must fail to compile.

#include <iostream>

struct S {
    const int x;
    int y;
    S(int a, int b) : x(a), y(b) {}
    S(S reloc rhs) : x(reloc rhs.x), y(reloc rhs.y) {}
    ~S() { std::cout << "~S()" << std::endl; }
};

int main(int, char**)
{
    S a(1, 2), b(3, 4);
    a = reloc b;
    return 0;
}

////// BUILD FAILURE
// simple-relocation-018.cpp:18:7: error: object of type 'S' cannot be assigned because its copy assignment operator is implicitly deleted
//    18 |     a = reloc b;
//       |       ^
// simple-relocation-018.cpp:8:15: note: copy assignment operator of 'S' is implicitly deleted because field 'x' is of const-qualified type 'const int'
//     8 |     const int x;
//       |               ^
// 1 error generated.
