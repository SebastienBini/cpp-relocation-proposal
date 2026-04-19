using T = int;
T getT() { return {}; }
void bar(T) {}
bool sometest(T) { return false; }
void do_smth(T) {}
void do_smth_else(T) {}

void reloc_showcase_06()
{
    const T var = getT();
    bool relocated = false;
    if (sometest(var))
    {
        bar(reloc var);
        relocated = true;
    }
    else
        do_smth(var); // OK
    // [...]
    if (!relocated)
        do_smth_else(var); // ill-formed: state of 'var' is A-R
}

////// BUILD FAILURE
// use-after-reloc-008.cpp:21:22: error: use of 'var' after it may have been relocated
//    21 |         do_smth_else(var); // ill-formed: state of 'var' is A-R
//       |                      ^
// 1 error generated.
