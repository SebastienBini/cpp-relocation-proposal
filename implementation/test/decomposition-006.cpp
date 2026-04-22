// Decomposed object used as a value (passed to function) is ill-formed.

struct S {
    int x;
    int y;
};

void sink(S) {}

int main(int, char**) {
    S reloc s;
    sink(s);
    (void)(s);
    return 0;
}

////// BUILD FAILURE
// decomposition-006.cpp:12:10: error: decomposed object 's' does not yield a value; access individual members ('obj.member') or base subobjects ('obj.base<B>') instead
//    12 |     sink(s);
//       |          ^
// decomposition-006.cpp:13:12: error: decomposed object 's' does not yield a value; access individual members ('obj.member') or base subobjects ('obj.base<B>') instead
//    13 |     (void)(s);
//       |            ^
// 2 errors generated.
