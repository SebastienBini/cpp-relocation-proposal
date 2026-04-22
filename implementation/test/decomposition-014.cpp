// base<> with a non-class type is ill-formed.

struct D {
    int dx;
};

int main(int, char**) {
    D reloc d;
    (void)d.base<int>;
    return 0;
}

////// BUILD FAILURE
