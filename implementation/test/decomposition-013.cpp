// base<> on cv-qualified base type is ill-formed.

struct B {
    int bx;
};

struct D : B {
    int dx;
};

int main(int, char**) {
    D reloc d;
    (void)d.base<const B>;
    return 0;
}

////// BUILD FAILURE
