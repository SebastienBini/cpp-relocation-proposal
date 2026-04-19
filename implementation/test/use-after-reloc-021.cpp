#include <exception>
#include <iostream>

using T = int;
T getT() { return {}; }
void foo() {}
void bar(T) {}

void reloc_showcase_19()
{
    T x = getT();
    try
    {
        foo();
        bar(reloc x);
    }
    catch (std::exception const& excp)
    {
        std::cerr << excp.what() << " for " << x << std::endl;
        // ill-formed: x state is alive or relocated
    }
}

////// BUILD FAILURE
// use-after-reloc-021.cpp:19:48: error: use of 'x' after it may have been relocated
//    19 |         std::cerr << excp.what() << " for " << x << std::endl;
//       |                                                ^
// 1 error generated.
