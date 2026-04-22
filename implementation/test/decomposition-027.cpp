// Decomposed object used as operand of assignment is ill-formed.

struct S {
    int x;
    int y;
};

int foo() {
    S reloc s;
    S other;
    other = s;
    return 0;
}

int bar() {
    S reloc s;
    S copy = s;
    return 0;
}

////// BUILD FAILURE
// decomposition-027.cpp:11:13: error: decomposed object 's' does not yield a value; access individual members ('obj.member') or base subobjects ('obj.base<B>') instead
//    11 |     other = s;
//       |             ^
// decomposition-027.cpp:17:14: error: decomposed object 's' does not yield a value; access individual members ('obj.member') or base subobjects ('obj.base<B>') instead
//    17 |     S copy = s;
//       |              ^
// 2 errors generated.
