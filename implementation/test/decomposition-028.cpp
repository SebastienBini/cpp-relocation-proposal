// Decomposed object used as initializer (implicit copy) is ill-formed.

struct S {
    int x;
    int y;
};

int main(int, char**) {
    S reloc s;
    S copy = s;
    return 0;
}

////// BUILD FAILURE
