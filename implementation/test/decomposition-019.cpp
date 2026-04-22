// base<> access on a non-decomposed object is ill-formed.

struct B {
    int bx;
};

struct D : B {
    int dx;
};

int main(int, char**) {
    D d;
    (void)d.base<B>;
    return 0;
}

////// BUILD FAILURE
