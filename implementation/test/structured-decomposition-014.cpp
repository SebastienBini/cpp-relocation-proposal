// Function-template `decompose<T>(T)` that uses structured bindings via
// the customized `operator reloc[]` protocol.  The template is
// instantiated for two different types, each with its own `operator
// reloc[]` returning a different inner aggregate.

#include "snoop.h"

struct A
{
    snoop u;
    snoop v;
    A() : u("A.u"), v("A.v") {}
    auto operator reloc[](this A reloc self) {
        struct R { snoop a, b; };
        return R{reloc self.u, reloc self.v};
    }
};

struct B
{
    snoop p;
    snoop q;
    B() : p("B.p"), q("B.q") {}
    auto operator reloc[](this B reloc self) {
        struct R { snoop a, b; };
        return R{reloc self.p, reloc self.q};
    }
};

template <class T>
void decompose(T t)
{
    auto [a, b] = reloc t;
    std::cout << "..." << std::endl;
    reloc a;
    std::cout << "..." << std::endl;
}

int main(int, char**)
{
    {
        A a;
        std::cout << "--- decompose<A> ---" << std::endl;
        decompose(reloc a);
        std::cout << "..." << std::endl;
    }
    std::cout << "===" << std::endl;
    {
        B b;
        std::cout << "--- decompose<B> ---" << std::endl;
        decompose(reloc b);
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// A.u() 0x1
// A.v() 0x2
// --- decompose<A> ---
// A.u(A.u&&) 0x3 <- 0x1
// A.v(A.v&&) 0x4 <- 0x2
// A.u(A.u&&) 0x5 <- 0x3
// A.v(A.v&&) 0x6 <- 0x4
// A.u(A.u reloc) 0x7 <- 0x5
// A.v(A.v reloc) 0x8 <- 0x6
// ~A.v() 0x4
// ~A.u() 0x3
// ...
// ~A.u() 0x7
// ...
// ~A.v() 0x8
// ~A.v() 0x2
// ~A.u() 0x1
// ...
// ===
// B.p() 0x9
// B.q() 0x10
// --- decompose<B> ---
// B.p(B.p&&) 0x3 <- 0x9
// B.q(B.q&&) 0x4 <- 0x10
// B.p(B.p&&) 0x5 <- 0x3
// B.q(B.q&&) 0x6 <- 0x4
// B.p(B.p reloc) 0x7 <- 0x5
// B.q(B.q reloc) 0x8 <- 0x6
// ~B.q() 0x4
// ~B.p() 0x3
// ...
// ~B.p() 0x7
// ...
// ~B.q() 0x8
// ~B.q() 0x10
// ~B.p() 0x9
// ...
