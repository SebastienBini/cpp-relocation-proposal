// Non-static member function calls on decomposed objects are ill-formed.

struct S {
    int x;
    void mutate() {}
    int get() const { return x; }
    void foo() {}
    void bar(int) {}
    void bar(double) {}
    virtual void vfunc() {}
};

int main(int, char**) {
    S reloc s;
    s.mutate();
    (void)s.get();
    constexpr auto pmf = &S::foo;
    (s.*pmf)();
    s.bar(1);
    s.vfunc();
    return 0;
}

////// BUILD FAILURE
// decomposition-005.cpp:15:7: error: member function calls are not permitted on decomposed objects; access individual members or base subobjects instead
//    15 |     s.mutate();
//       |       ^
// decomposition-005.cpp:16:13: error: member function calls are not permitted on decomposed objects; access individual members or base subobjects instead
//    16 |     (void)s.get();
//       |             ^
// decomposition-005.cpp:18:7: error: member function calls are not permitted on decomposed objects; access individual members or base subobjects instead
//    18 |     (s.*pmf)();
//       |       ^
// decomposition-005.cpp:19:7: error: member function calls are not permitted on decomposed objects; access individual members or base subobjects instead
//    19 |     s.bar(1);
//       |       ^
// decomposition-005.cpp:20:7: error: member function calls are not permitted on decomposed objects; access individual members or base subobjects instead
//    20 |     s.vfunc();
//       |       ^
// 5 errors generated.
