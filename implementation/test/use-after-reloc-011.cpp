using T = int;
T getT() { return {}; }
void do_smth(T) {}

void reloc_showcase_09()
{
    const T var = getT();
    for (int i = 0; i != 1; ++i)
        do_smth(reloc var); // ill-formed: state of 'var' is A-R (on second iteration)
}

////// BUILD FAILURE
// use-after-reloc-011.cpp:9:17: error: use of 'var' after it may have been relocated
//     9 |         do_smth(reloc var); // ill-formed: state of 'var' is A-R (on second iteration)
//       |                 ^
// 1 error generated.
