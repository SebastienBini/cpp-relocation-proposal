#include "snoop.h"
#include "box.h"

int main(int, char**)
{
    box<snoop> const b{"box<snoop>"};
    auto reloc c = reloc b;
    return 0;
}

////// BUILD FAILURE
// box-005.cpp:7:16: error: cannot decompose an object of type 'box<snoop>' with a user-provided destructor from outside the class or its friends; move individual members instead
//     7 |     auto reloc c = reloc b;
//       |                ^
// 1 error generated.
