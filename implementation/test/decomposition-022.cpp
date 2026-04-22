// Virtual member function call on decomposed object is ill-formed.

struct S {
    int x;
    virtual void vfunc() {}
};

int main(int, char**) {
    S reloc s;
    s.vfunc();
    return 0;
}

////// BUILD FAILURE
