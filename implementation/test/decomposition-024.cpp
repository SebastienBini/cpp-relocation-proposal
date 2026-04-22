// Cannot decompose an object with a user-provided destructor from outside
// the class or its friends.

struct S {
    int x;
    ~S() {}
};

int main(int, char**) {
    S reloc s;
    return 0;
}

////// BUILD FAILURE
