using T = int;
bool sometest(T) { return false; }
bool othertest(T) { return false; }
void do_smth(T) {}
void foo() {}

void reloc_showcase_17()
{
    T x;
    if (sometest(x) || sometest(reloc x)) // well-formed: x state is A-R
    {
        do_smth(x); // ill-formed: x state is A-R
    }
    else if (othertest(x)) // ill-formed: x state is A-R
    {
        foo();
    }
}

////// BUILD FAILURE
// use-after-reloc-019.cpp:14:24: error: use of 'x' after it has been relocated
//    14 |     else if (othertest(x)) // ill-formed: x state is A-R
//       |                        ^
// use-after-reloc-019.cpp:12:17: error: use of 'x' after it may have been relocated
//    12 |         do_smth(x); // ill-formed: x state is A-R
//       |                 ^
// 2 errors generated.
