// Three `operator reloc[]` overloads selected by the value-category /
// const-ness of the source: lvalue picks `() const&` (copy semantics),
// non-const xvalue picks `() &&` (move-from), `reloc` source picks
// `(this S reloc self)` (full-strength relocation).  The snoop output
// shows which one ran.

#include "snoop.h"

struct Result { snoop a, b; };

struct S
{
    snoop u;
    snoop v;
    S() : u("U"), v("V") {}
    S(S&& o) : u(static_cast<snoop&&>(o.u)), v(static_cast<snoop&&>(o.v)) {}
    S(S const& o) : u(o.u), v(o.v) {}
    S(S reloc o) : u(reloc o.u), v(reloc o.v) {}

    Result operator reloc[]() const&
    {
        std::cout << "op reloc[]() const&" << std::endl;
        return Result{u, v};            // copies
    }
    Result operator reloc[]() &&
    {
        std::cout << "op reloc[]() &&" << std::endl;
        return Result{static_cast<snoop&&>(u), static_cast<snoop&&>(v)};
    }
    Result operator reloc[](this S reloc self)
    {
        std::cout << "op reloc[](this S reloc)" << std::endl;
        return Result{reloc self.u, reloc self.v};
    }
};

int main(int, char**)
{
    {
        S s;
        std::cout << "--- lvalue ---" << std::endl;
        auto [a, b] = s;
        std::cout << "..." << std::endl;
    }
    std::cout << "===" << std::endl;
    {
        S s;
        std::cout << "--- xvalue ---" << std::endl;
        auto [a, b] = static_cast<S&&>(s);
        std::cout << "..." << std::endl;
    }
    std::cout << "===" << std::endl;
    {
        S s;
        std::cout << "--- reloc ---" << std::endl;
        auto [a, b] = reloc s;
        std::cout << "..." << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// U() 0x1
// V() 0x2
// --- lvalue ---
// op reloc[]() const&
// U(U const&); 0x3 <- 0x1
// V(V const&); 0x4 <- 0x2
// ...
// ~V() 0x4
// ~U() 0x3
// ~V() 0x2
// ~U() 0x1
// ===
// U() 0x5
// V() 0x6
// --- xvalue ---
// op reloc[]() &&
// U(U&&) 0x7 <- 0x5
// V(V&&) 0x8 <- 0x6
// ...
// ~V() 0x8
// ~U() 0x7
// ~V() 0x6
// ~U() 0x5
// ===
// U() 0x9
// V() 0x10
// --- reloc ---
// op reloc[](this S reloc)
// U(U reloc) 0x11 <- 0x9
// V(V reloc) 0x12 <- 0x10
// ...
// ~V() 0x12
// ~U() 0x11
