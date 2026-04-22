// Cannot use a decomposed reference type.

struct S {
    int x;
};

void f(S& reloc r) {
    (void)r.x;
}
void f(S* reloc r) {
    (void)r.x;
}
void f(S reloc r[]) {
    (void)r.x;
}

int main(int, char**) {
    return 0;
}

////// BUILD FAILURE
// decomposition-029.cpp:7:11: error: decomposed object may not have a reference type; 'S &' is a reference
//     7 | void f(S& reloc r) {
//       |           ^
// decomposition-029.cpp:10:11: error: decomposed object must have a non-union class type; 'S *' is not a class type
//    10 | void f(S* reloc r) {
//       |           ^
// decomposition-029.cpp:13:10: error: decomposed object must have a non-union class type; 'S *' is not a class type
//    13 | void f(S reloc r[]) {
//       |          ^
// 3 errors generated.
