// base<> on cv-qualified base type is ill-formed.

struct B {
    int bx;
};

struct Unrelated {
    int ux;
};

struct D : B {
    int dx;
};

int main(int, char**) {
    D reloc d;
    (void)d.base<D>;
    (void)d.base<const B>;
    (void)d.base<int>;
    (void)d.base<Unrelated>;
    (void)d.base<xxx>;
    return 0;
}

////// BUILD FAILURE
// decomposition-009.cpp:17:13: error: 'D' is not a base class of 'D'
//    17 |     (void)d.base<D>;
//       |             ^
// decomposition-009.cpp:18:17: error: base-class-type in '.base<>' must not be cv-qualified
//    18 |     (void)d.base<const B>;
//       |                 ^
// decomposition-009.cpp:19:17: error: base-class-type 'int' is not a class type
//    19 |     (void)d.base<int>;
//       |                 ^
// decomposition-009.cpp:20:13: error: 'Unrelated' is not a base class of 'D'
//    20 |     (void)d.base<Unrelated>;
//       |             ^
// decomposition-009.cpp:21:18: error: unknown type name 'xxx'
//    21 |     (void)d.base<xxx>;
//       |                  ^
// 5 errors generated.
