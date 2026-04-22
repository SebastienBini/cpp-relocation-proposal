// Overloaded member function call on decomposed object is ill-formed.

struct S {
    int x;
    void foo(int) {}
    void foo(double) {}
};

int main(int, char**) {
    S reloc s;
    s.foo(1);
    return 0;
}

////// BUILD FAILURE
