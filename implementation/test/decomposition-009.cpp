// Decomposed object used as a value in parenthesized expression is ill-formed.

struct S {
    int x;
    int y;
};

int main(int, char**) {
    S reloc s;
    (void)(s);
    return 0;
}

////// BUILD FAILURE
