/// Trivially relocatable, relocatable-non-movable, user-provided destructor.
/// A has no user-declared copy/reloc ctors (trivially relocatable) but move
/// is deleted -> relocatable-non-movable -> early-destructible parameter.
/// Chain: main -> bar -> baz, forwarding by value with reloc at each step.
/// Only one destructor call (no moved-from leftover).

#include <iostream>

struct A {
    int value;

    A(int v) : value(v) { std::cout << "A(" << value << ") " << this << std::endl; }
    A(A const&) = default;
    A(A&& rhs) = delete;
    A(A reloc) = default;
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
// ~A(42) 0x1
// ---
