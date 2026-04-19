using T = int;
T getT() { return {}; }
void bar(T) {}
bool sometest(T) { return false; }
void do_smth(T) {}

void reloc_showcase_04()
{
    const T var = getT();
    if (sometest(var))
        bar(reloc var); // #1
    else
        do_smth(var); // OK
    // [...]
    do_smth(var); // #2, ill-formed: state of 'var' is A-R
}

////// BUILD FAILURE
// use-after-reloc-006.cpp:15:13: error: use of 'var' after it may have been relocated
//    15 |     do_smth(var); // #2, ill-formed: state of 'var' is A-R
//       |             ^
// 1 error generated.
