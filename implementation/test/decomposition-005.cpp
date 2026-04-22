// Non-static member function calls on decomposed objects are ill-formed.

struct S {
    int x;
    void mutate() {}
    int get() const { return x; }
};

int main(int, char**) {
    S reloc s;
    s.mutate();
    return 0;
}

////// BUILD FAILURE
