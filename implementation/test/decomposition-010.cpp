// Qualified member access on decomposed objects is not supported.

struct B {
    int bx;
};

struct D : B {
    int dx;
};

int main(int, char**) {
    D reloc d;
    (void)d.B::bx;
    return 0;
}

////// BUILD FAILURE
