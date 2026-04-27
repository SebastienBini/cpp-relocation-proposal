/// Reloc ctor body throws: the most derived class's reloc ctor body throws
/// after all subobjects are initialized.
/// Uses "return param" for a reloc-only type to force the reloc ctor to run.

#include <iostream>
#include <stdexcept>
#include "snoop.h"

struct A : public snoop {
    snoop m;
    A() : snoop("snoop"), m("m") { std::cout << "A() " << this << std::endl; }
    A(A const&) = delete;
    A(A&&) = delete;
    A(A reloc rhs) : snoop(reloc rhs.base<snoop>), m(reloc rhs.m) {
        std::cout << "A(A reloc) " << this << " <- " << rhs.this << std::endl;
        throw std::runtime_error("boom");
    }
    ~A() { std::cout << "~A() " << this << std::endl; }
};

A bar(A a) { return a; }

int main(int, char**)
{
    A a;
    std::cout << "---" << std::endl;
    try { A b = bar(reloc a); }
    catch (...) { std::cout << "caught" << std::endl; }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// snoop() 0x1
// m() 0x2
// A() 0x1
// ---
// snoop(snoop reloc) 0x3 <- 0x1
// m(m reloc) 0x4 <- 0x2
// A(A reloc) 0x3 <- 0x1
// ~m() 0x4
// ~snoop() 0x3
// caught
// ---
