/// Trivially relocatable with user-provided destructor: return by value.
/// main -> bar(reloc a) -> bar returns reloc obj -> b receives result.
/// A is trivially relocatable (no user-declared copy/move/reloc ctors).

#include <iostream>

struct A {
    int value;

    A(int v) : value(v) { std::cout << "A(" << value << ") " << this << std::endl; }
    ~A() { std::cout << "~A(" << value << ") " << this << std::endl; }
};

A bar(A obj)
{
    std::cout << "bar " << obj.value << std::endl;
    return reloc obj;
}

int main(int, char**)
{
    A a{42};
    std::cout << "---" << std::endl;
    A b = bar(reloc a);
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// A(42) 0x1
// ---
// bar 42
// ~A(42) 0x1
// ---
// ~A(42) 0x2
