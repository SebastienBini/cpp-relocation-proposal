using T = int;
T getT() { return {}; }
bool sometest(T) { return false; }
void do_smth(T) {}

void reloc_showcase_13()
{
    const T var = getT();
from: // #1
    if (sometest(var)) // #2, ill-formed: var state is A-R after the jump
    {
        do_smth(var); // #3, ill-formed: var state is A-R after the jump
        return;
    }
    else
    {
        do_smth(reloc var); // #4, ill-formed: var state is A-R after the jump
    }
    // #5
    goto from;
}

////// BUILD FAILURE
// use-after-reloc-015.cpp:17:17: error: use of 'var' after it may have been relocated
//    17 |         do_smth(reloc var); // #4, ill-formed: var state is A-R after the jump
//       |                 ^
// use-after-reloc-015.cpp:12:17: error: use of 'var' after it may have been relocated
//    12 |         do_smth(var); // #3, ill-formed: var state is A-R after the jump
//       |                 ^
// use-after-reloc-015.cpp:10:18: error: use of 'var' after it may have been relocated
//    10 |     if (sometest(var)) // #2, ill-formed: var state is A-R after the jump
//       |                  ^
// 3 errors generated.
