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
// decomposition-024.cpp:10:7: error: cannot decompose an object of type 'S' with a user-provided destructor from outside the class or its friends; move individual members instead
//    10 |     S reloc s;
//       |       ^
// 1 error generated.
