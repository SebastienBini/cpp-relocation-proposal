// Cannot relocate a virtual base subobject of a decomposed object.

struct V {
    int vx;
    V() = default;
    V(V reloc) = default;
    ~V() = default;
};

struct D : virtual V {
    int dx;
    D() = default;
    D(D reloc) = default;
    ~D() = default;
};

void sink(V reloc) {}

int main(int, char**) {
    D reloc d;
    sink(reloc d.base<V>);
    return 0;
}

////// BUILD FAILURE
// decomposition-025.cpp:21:10: error: cannot relocate virtual base 'V' of a decomposed object
//    21 |     sink(reloc d.base<V>);
//       |          ^
// 1 error generated.
