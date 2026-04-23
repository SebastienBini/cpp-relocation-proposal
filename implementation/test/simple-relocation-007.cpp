/// Trivially relocatable with user-provided destructor that logs.
/// A has no user-declared copy/move/reloc ctors -> implicitly trivially reloc.
/// Chain: main -> bar -> baz, forwarding by value with reloc at each step.

#include <iostream>

struct A {
    int value;

    A(int v) : value(v) { std::cout << "A(" << value << ") " << this << std::endl; }
    ~A() { std::cout << "~A(" << value << ") " << this << std::endl; }
};

void baz(A obj)
{
    std::cout << "baz " << obj.value << std::endl;
}

void bar(A obj)
{
    std::cout << "bar " << obj.value << std::endl;
    baz(reloc obj);
}

int main(int, char**)
{
    A a{42};
    std::cout << "---" << std::endl;
    bar(reloc a);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// A(42) 0x1
// ---
// bar 42
// baz 42
// ~A(42) 0x2
// ~A(42) 0x1
// ---
