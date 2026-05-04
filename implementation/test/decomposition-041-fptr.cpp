/// Decomposing function with two parameters invoked through a function
/// pointer. Both parameter types are non-RNM (have a move constructor) and
/// the move constructor of the last parameter throws while the twin entry
/// is moving the arguments into its parameter slots. Param1 has already
/// been fully constructed in the twin frame at that point and must be
/// destroyed as a whole object during stack unwinding (the canonical
/// decomposing entry is never reached, so no decomposed cleanup runs).

#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include "snoop.h"

struct T;

struct A : public snoop {
    snoop ax;
    A() : snoop("Abase"), ax("ax") { std::cout << "A() " << this << std::endl; }
    A(A const& rhs) : snoop(rhs), ax(rhs.ax) { std::cout << "A(A const&) " << this << " <- " << &rhs << std::endl; }
    A(A&& rhs) : snoop(std::move(rhs)), ax(std::move(rhs.ax)) { std::cout << "A(A&&) " << this << " <- " << &rhs << std::endl; }
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>), ax(reloc rhs.ax) { std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl; }
    ~A() { std::cout << "~A() " << this << std::endl; }
    friend void decomp(A reloc a, T reloc t);
};

struct T : public snoop {
    snoop tx;
    T() : snoop("Tbase"), tx("tx") { std::cout << "T() " << this << std::endl; }
    T(T const& rhs) : snoop(rhs), tx(rhs.tx) { std::cout << "T(T const&) " << this << " <- " << &rhs << std::endl; }
    T(T&& rhs) : snoop(std::move(rhs)), tx(std::move(rhs.tx)) {
        std::cout << "T(T&&) " << this << " <- " << &rhs << std::endl;
        throw std::runtime_error{"T move throws"};
    }
    T(T reloc rhs) : snoop(reloc rhs.base<snoop>), tx(reloc rhs.tx) { std::cout << "T(T reloc) " << this << " <- " << rhs.this << std::endl; }
    ~T() { std::cout << "~T() " << this << std::endl; }
    friend void decomp(A reloc a, T reloc t);
};

void decomp(A reloc a, T reloc t)
{
    std::cout << "decomp ---" << std::endl;
    snoop s = reloc a.base<snoop>;
    snoop u = reloc t.base<snoop>;
    (void)s;
    (void)u;
    std::cout << "decomp ---" << std::endl;
}

int main(int, char**)
{
    void (*fp)(A, T) = &decomp;
    A a;
    T t;
    std::cout << "main ---" << std::endl;
    try { fp(std::move(a), std::move(t)); }
    catch (std::exception const& e) { std::cout << "caught: " << e.what() << std::endl; }
    std::cout << "main ---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// Abase() 0x1
// ax() 0x2
// A() 0x1
// Tbase() 0x3
// tx() 0x4
// T() 0x3
// main ---
// Abase(Abase&&) 0x5 <- 0x1
// ax(ax&&) 0x6 <- 0x2
// A(A&&) 0x5 <- 0x1
// Tbase(Tbase&&) 0x7 <- 0x3
// tx(tx&&) 0x8 <- 0x4
// T(T&&) 0x7 <- 0x3
// ~tx() 0x8
// ~Tbase() 0x7
// ~A() 0x5
// ~ax() 0x6
// ~Abase() 0x5
// caught: T move throws
// main ---
// ~T() 0x3
// ~tx() 0x4
// ~Tbase() 0x3
// ~A() 0x1
// ~ax() 0x2
// ~Abase() 0x1
