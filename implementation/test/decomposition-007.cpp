// Qualified member access on decomposed objects is not supported.

struct B {
    int bx;
};

struct D : B {
    int dx;
};

int main(int, char**) {
    D reloc d;
    (void)d.B::bx;
    return 0;
}

////// BUILD FAILURE
// decomposition-007.cpp:13:13: error: qualified member access ('obj.Qualifier::member') is not supported for decomposed objects; use 'obj.base<Qualifier>.member' instead
//    13 |     (void)d.B::bx;
//       |             ^
// 1 error generated.
