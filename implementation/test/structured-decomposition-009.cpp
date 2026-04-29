#include "snoop.h"
#include "box.h"

template <int = 0>
void run() {
    box<snoop> arr[3] = {box<snoop>("a"), box<snoop>("b"), box<snoop>("c")};
    std::cout << "---" << std::endl;
    auto [...a] = reloc arr;
    std::cout << "..." << std::endl;
    reloc a...[1];
    std::cout << "..." << std::endl;
    auto d = reloc a...[0];
    std::cout << "..." << std::endl;
}

int main(int, char**)
{
    run();
    return 0;
}

////// BUILD FAILURE
// structured-decomposition-009.cpp:10:11: error: operand of 'reloc' must be a local variable
//    10 |     reloc a...[1];
//       |           ^~~~~~~
// structured-decomposition-009.cpp:18:5: note: in instantiation of function template specialization 'run<0>' requested here
//    18 |     run();
//       |     ^
// structured-decomposition-009.cpp:12:20: error: operand of 'reloc' must be a local variable
//    12 |     auto d = reloc a...[0];
//       |                    ^~~~~~~
// 2 errors generated.
