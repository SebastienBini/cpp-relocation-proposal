struct T {
    T() = default;
    T(T const&) = default;
    T operator+(T const&) const { return {}; }
    T operator[](T const&) const { return {}; }
    operator bool() const { return false; }
};

bool test() { return false; }
void foo(T) {}
void foo(T, T) {}

void reloc_showcase_18()
{
    // unspecified evaluation order
    { T x; auto y = reloc x + reloc x; } // ill-formed
    { T x; auto y = x + reloc x; }       // ill-formed
    { T x; auto y = reloc x + x; }       // ill-formed
    { T x; auto y = x[reloc x]; }        // ill-formed

    // sequenced evaluation order
    { T x; auto y = (x, reloc x); }      // well-formed
    { T x; auto y = (reloc x, x); }      // ill-formed
    { T x; auto y = x or reloc x; }      // well-formed
    { T x; auto y = reloc x or x; }      // ill-formed

    // unspecified evaluation order with ternary
    { T x; foo(x, reloc x); }                        // ill-formed
    { T x; foo(x, test() ? reloc x : x); }           // ill-formed
    { T x; foo(test() ? reloc x : x); }              // well-formed
}

////// BUILD FAILURE
// use-after-reloc-020.cpp:16:31: error: multiple relocations of 'x' are unsequenced
//    16 |     { T x; auto y = reloc x + reloc x; } // ill-formed
//       |                               ^
// use-after-reloc-020.cpp:16:21: note: relocation of 'x' is here
//    16 |     { T x; auto y = reloc x + reloc x; } // ill-formed
//       |                     ^
// use-after-reloc-020.cpp:17:21: error: use of 'x' is unsequenced with its relocation
//    17 |     { T x; auto y = x + reloc x; }       // ill-formed
//       |                     ^
// use-after-reloc-020.cpp:17:25: note: relocation of 'x' is here
//    17 |     { T x; auto y = x + reloc x; }       // ill-formed
//       |                         ^
// use-after-reloc-020.cpp:18:31: error: use of 'x' is unsequenced with its relocation
//    18 |     { T x; auto y = reloc x + x; }       // ill-formed
//       |                               ^
// use-after-reloc-020.cpp:18:21: note: relocation of 'x' is here
//    18 |     { T x; auto y = reloc x + x; }       // ill-formed
//       |                     ^
// use-after-reloc-020.cpp:19:21: error: use of 'x' is unsequenced with its relocation
//    19 |     { T x; auto y = x[reloc x]; }        // ill-formed
//       |                     ^
// use-after-reloc-020.cpp:19:23: note: relocation of 'x' is here
//    19 |     { T x; auto y = x[reloc x]; }        // ill-formed
//       |                       ^
// use-after-reloc-020.cpp:23:31: error: use of 'x' after it has been relocated
//    23 |     { T x; auto y = (reloc x, x); }      // ill-formed
//       |                               ^
// use-after-reloc-020.cpp:23:22: note: relocated here
//    23 |     { T x; auto y = (reloc x, x); }      // ill-formed
//       |                      ^
// use-after-reloc-020.cpp:25:32: error: use of 'x' after it has been relocated
//    25 |     { T x; auto y = reloc x or x; }      // ill-formed
//       |                                ^
// use-after-reloc-020.cpp:25:21: note: relocated here
//    25 |     { T x; auto y = reloc x or x; }      // ill-formed
//       |                     ^
// use-after-reloc-020.cpp:28:16: error: use of 'x' is unsequenced with its relocation
//    28 |     { T x; foo(x, reloc x); }                        // ill-formed
//       |                ^
// use-after-reloc-020.cpp:28:19: note: relocation of 'x' is here
//    28 |     { T x; foo(x, reloc x); }                        // ill-formed
//       |                   ^
// use-after-reloc-020.cpp:29:16: error: use of 'x' is unsequenced with its relocation
//    29 |     { T x; foo(x, test() ? reloc x : x); }           // ill-formed
//       |                ^
// use-after-reloc-020.cpp:29:28: note: relocation of 'x' is here
//    29 |     { T x; foo(x, test() ? reloc x : x); }           // ill-formed
//       |                            ^
// use-after-reloc-020.cpp:22:22: warning: left operand of comma operator has no effect [-Wunused-value]
//    22 |     { T x; auto y = (x, reloc x); }      // well-formed
//       |                      ^
// use-after-reloc-020.cpp:23:22: warning: left operand of comma operator has no effect [-Wunused-value]
//    23 |     { T x; auto y = (reloc x, x); }      // ill-formed
//       |                      ^
// 2 warnings and 8 errors generated.
