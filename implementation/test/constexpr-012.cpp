/// constexpr non-trivial defaulted reloc ctor: inner type has user-provided
/// reloc ctor (+10), outer defaults.
/// Mandatory relocation elision (§reloc-elision-mandatory): local-to-local
/// reloc elides the ctor both at compile time and at runtime.

#include <iostream>
#include "cxlog.h"

struct Inner {
    int v;
    constexpr Inner(int x) : v(x) {}
    constexpr Inner(Inner reloc src) : v(src.v + 10) {}
    constexpr ~Inner() {}
};

struct Outer {
    Inner a;
    constexpr Outer(int x) : a(x) {}
    constexpr Outer(Outer reloc) = default;
    constexpr ~Outer() {}
};

constexpr int test() {
    Outer o(5);
    Outer p = reloc o;
    return p.a.v;
}

// Mandatory elision: reloc ctor is not called.
static_assert(test() == 5, "");

// cxlog trace: Inner and Outer ctors/dtors, no reloc ctor.
struct InnerL {
    cxlog* log; int v;
    constexpr InnerL(cxlog& l, int x) : log(&l), v(x)
        { log->record('I', cxlog::CTOR, this); }
    constexpr InnerL(InnerL reloc src) : log(src.log), v(src.v + 10)
        { log->record('I', cxlog::RELOC, this, &src); }
    constexpr ~InnerL() { log->record('I', cxlog::DTOR, this); }
};

struct OuterL {
    InnerL a;
    cxlog* log;
    constexpr OuterL(cxlog& l, int x) : a(l, x), log(&l)
        { log->record('O', cxlog::CTOR, this); }
    constexpr OuterL(OuterL reloc) = default;
    constexpr ~OuterL() { log->record('O', cxlog::DTOR, this); }
};

constexpr int test_log() {
    cxlog log;
    { OuterL o(log, 5); OuterL p = reloc o; (void)p; }
    return log.match_ops("I(); O(); ~O(); ~I()");
}
static_assert(test_log() == 1, "mandatory elision: no inner/outer reloc ctor");

int main()
{
    constexpr int ct = test();
    int rt = test();
    std::cout << "compile_time: " << ct << std::endl;
    std::cout << "runtime: " << rt << std::endl;
    // cxlog trace
    cxlog log;
    { OuterL o(log, 5); OuterL p = reloc o; (void)p; }
    std::cout << "trace: "; log.dump(); std::cout << std::endl;
    return 0;
}

////// BUILD SUCCESS
// compile_time: 5
// runtime: 5
// trace: I() 0x1; O() 0x1; ~O() 0x1; ~I() 0x1
