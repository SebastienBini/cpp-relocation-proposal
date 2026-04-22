// Cannot relocate a decomposed object as a whole.

struct S {
    int x;
    int y;
};

void sink(S reloc) {}

int main(int, char**) {
    S reloc s;
    sink(reloc s);
    return 0;
}

////// BUILD FAILURE
// decomposition-016.cpp:12:16: error: cannot relocate a decomposed object as a whole; use 'reloc obj.member' to relocate individual members one at a time
//    12 |     sink(reloc s);
//       |                ^
// 1 error generated.
