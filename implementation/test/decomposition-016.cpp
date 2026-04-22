// Cannot relocate a decomposed object as a whole.

struct S {
    int x;
    int y;
};

void sink(S reloc) {}

int main(int, char**) {
    S reloc s;
    sink(reloc s);
    return 0;
}

////// BUILD FAILURE
