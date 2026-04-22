// .this access on a non-decomposed object is ill-formed.

struct S {
    int x;
};

int main(int, char**) {
    S s;
    (void)s.this;
    return 0;
}

////// BUILD FAILURE
// decomposition-020.cpp:9:11: error: 'obj.this' requires 's' to be a decomposed object ('T reloc name'); 's' is not a decomposed variable
//     9 |     (void)s.this;
//       |           ^
// 1 error generated.
