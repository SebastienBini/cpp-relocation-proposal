// reloc on a data member of a non-decomposed object is ill-formed.

struct S {
    int x;
    int y;
};

int main(int, char**) {
    S s;
    (void)(reloc s.x);
    return 0;
}

////// BUILD FAILURE
