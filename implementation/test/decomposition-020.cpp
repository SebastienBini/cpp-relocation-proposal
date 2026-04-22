// .this access on a non-decomposed object is ill-formed.

struct S {
    int x;
};

int main(int, char**) {
    S s;
    (void)s.this;
    return 0;
}

////// BUILD FAILURE
