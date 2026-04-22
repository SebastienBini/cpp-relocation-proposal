// Decomposed object used as a value (passed to function) is ill-formed.

struct S {
    int x;
};

void sink(S) {}

int main(int, char**) {
    S reloc s;
    sink(s);
    return 0;
}

////// BUILD FAILURE
