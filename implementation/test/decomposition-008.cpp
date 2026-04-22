// Cannot decompose a union type.

union U {
    int a;
    float b;
};

int main(int, char**) {
    int reloc x = 42;
    U reloc u;
    return 0;
}

////// BUILD FAILURE
// decomposition-008.cpp:9:9: error: decomposed object must have a non-union class type; 'int' is not a class type
//     9 |     int reloc x = 42;
//       |         ^
// decomposition-008.cpp:10:7: error: decomposed object must have a non-union class type; 'U' is a union
//    10 |     U reloc u;
//       |       ^
// 2 errors generated.
