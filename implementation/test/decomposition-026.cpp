// Cannot relocate an indirect (non-direct) base subobject.

struct A {
    int ax;
    A() = default;
    A(A reloc) = default;
    ~A() = default;
};

struct B : A {
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

void sink(A reloc) {}

int main(int, char**) {
    D reloc d;
    sink(reloc d.base<A>);
    return 0;
}

////// BUILD FAILURE
// decomposition-026.cpp:28:10: error: cannot relocate indirect base 'A'; only direct bases of the decomposed type may be relocated
//    28 |     sink(reloc d.base<A>);
//       |          ^
// 1 error generated.
