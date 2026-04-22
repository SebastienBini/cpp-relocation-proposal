// Pointer-to-member access on decomposed object requires constant-evaluated pointer.

struct S {
    int x;
    int y;
};

void f(S reloc s, int S::* p) {
    (void)(s.*p);
}

int main(int, char**) {
    return 0;
}

////// BUILD FAILURE
// decomposition-017.cpp:9:13: error: pointer-to-member access on decomposed object requires a constant-evaluated pointer; 'int S::*' is not constant-evaluated
//     9 |     (void)(s.*p);
//       |             ^ ~
// 1 error generated.
