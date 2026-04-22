// base<> access on a non-decomposed object is ill-formed.

struct B {
    int bx;
};

struct D : B {
    int dx;
};

int main(int, char**) {
    D d;
    (void)d.base<B>;
    return 0;
}

////// BUILD FAILURE
// decomposition-019.cpp:13:11: error: 'base<>' access requires a decomposed object ('T reloc name'); ''d'' is not a decomposed object
//    13 |     (void)d.base<B>;
//       |           ^
// 1 error generated.
