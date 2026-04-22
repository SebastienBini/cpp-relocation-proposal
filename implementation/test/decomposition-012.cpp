// Cannot decompose a union type.

union U {
    int a;
    float b;
};

int main(int, char**) {
    U reloc u;
    return 0;
}

////// BUILD FAILURE
