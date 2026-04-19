using T = int;
T getT() { return {}; }
void do_smth(T) {}

void reloc_showcase_10()
{
    const T var = getT();
    for (int i = 0; i != 10; ++i)
    {
        if (i == 9)
            do_smth(reloc var); // ill-formed: state of 'var' is A-R (on second iteration)
        else
            do_smth(var); // ill-formed: state of 'var' is A-R (on second iteration)
    }
}

////// BUILD FAILURE
// use-after-reloc-012.cpp:13:21: error: use of 'var' after it may have been relocated
//    13 |             do_smth(var); // ill-formed: state of 'var' is A-R (on second iteration)
//       |                     ^
// use-after-reloc-012.cpp:11:21: error: use of 'var' after it may have been relocated
//    11 |             do_smth(reloc var); // ill-formed: state of 'var' is A-R (on second iteration)
//       |                     ^
// 2 errors generated.
