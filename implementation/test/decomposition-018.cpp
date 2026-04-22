// reloc of a member declared in a base class is ill-formed;
// must use reloc obj.base<B> instead.

struct B {
    int bx;
    B() = default;
    B(B reloc) = default;
    ~B() = default;
};

struct D : B {
    int dx;
    D() = default;
    D(D reloc) = default;
    ~D() = default;
};

void sink(int) {}

int main(int, char**) {
    D reloc d;
    sink(reloc d.bx);
    return 0;
}

////// BUILD FAILURE
