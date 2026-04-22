// Decomposed object used as operand of assignment is ill-formed.

struct S {
    int x;
    int y;
};

int main(int, char**) {
    S reloc s;
    S other;
    other = s;
    return 0;
}

////// BUILD FAILURE
