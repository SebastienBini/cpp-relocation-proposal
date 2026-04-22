// Const member function calls on decomposed objects are also ill-formed.

struct S {
    int x;
    int get() const { return x; }
};

int main(int, char**) {
    S reloc s;
    (void)s.get();
    return 0;
}

////// BUILD FAILURE
