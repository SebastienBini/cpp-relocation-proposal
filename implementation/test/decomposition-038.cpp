#include <iostream>

struct S {};

void foo(S reloc) {}

template <void (*F)(S)>
void tpl() {}

int main()
{
    void (*a)(S) = &foo;
    static_assert(std::is_same_v<decltype(&foo), void (*)(S)>);
    tpl<&foo>();
    return 0;
}

////// BUILD SUCCESS
