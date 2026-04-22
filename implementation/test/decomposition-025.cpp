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
