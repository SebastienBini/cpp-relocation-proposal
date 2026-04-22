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
