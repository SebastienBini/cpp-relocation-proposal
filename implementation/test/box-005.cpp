#include "snoop.h"
#include "box.h"

int main(int, char**)
{
    box<snoop> const b{"box<snoop>"};
    auto reloc c = reloc b;
    return 0;
}

////// BUILD FAILURE
// box-005.cpp:7:16: error: cannot decompose type 'box<snoop>': private member '_ptr' is not accessible
//     7 |     auto reloc c = reloc b;
//       |                ^
// /workspace/llvm-project/P2785/implementation/test/box.h:38:8: note: declared private here
//    38 |     T* _ptr;
//       |        ^
// 1 error generated.
