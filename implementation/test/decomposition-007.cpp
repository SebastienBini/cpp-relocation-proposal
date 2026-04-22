// Pointer-to-member-function calls on decomposed objects are ill-formed.

struct S {
    int x;
    void foo() {}
};

int main(int, char**) {
    S reloc s;
    constexpr auto pmf = &S::foo;
    (s.*pmf)();
    return 0;
}

////// BUILD FAILURE
