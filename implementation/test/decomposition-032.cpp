// Cannot use a decomposed reference type.

struct S {
    int x;
};

void f(S& reloc r) {
    (void)r.x;
}

int main(int, char**) {
    return 0;
}

////// BUILD FAILURE
