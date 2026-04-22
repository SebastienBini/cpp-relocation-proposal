// base<> with a type that is not a base class is ill-formed.

struct B {
    int bx;
};

struct Unrelated {
    int ux;
};

struct D : B {
    int dx;
};

int main(int, char**) {
    D reloc d;
    (void)d.base<Unrelated>;
    return 0;
}

////// BUILD FAILURE
