// reloc on a data member of a non-decomposed object is ill-formed.

struct S {
    int x;
    int y;
};

int main(int, char**) {
    S s;
    (void)(reloc s.x);
    return 0;
}

////// BUILD FAILURE
// decomposition-021.cpp:10:18: error: 'reloc' on a class data member requires the containing object to be declared as a decomposed object ('T reloc name')
//    10 |     (void)(reloc s.x);
//       |                  ^
// 1 error generated.
