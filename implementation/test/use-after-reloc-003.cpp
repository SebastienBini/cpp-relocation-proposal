using T = int;
T getT();
void bar(T);
bool sometest(T);
void do_smth(T);

void reloc_showcase_01()
{
    const T var = getT();
    bar(reloc var);
    if (sometest(var)) // ill-formed: state of 'var' is relocated
        do_smth(var); // ill-formed: state of 'var' is relocated
}

////// BUILD FAILURE
// use-after-reloc-003.cpp:12:17: error: use of 'var' after it has been relocated
//    12 |         do_smth(var); // ill-formed: state of 'var' is relocated
//       |                 ^
// use-after-reloc-003.cpp:11:18: error: use of 'var' after it has been relocated
//    11 |     if (sometest(var)) // ill-formed: state of 'var' is relocated
//       |                  ^
// use-after-reloc-003.cpp:10:9: note: relocated here
//    10 |     bar(reloc var);
//       |         ^
// 2 errors generated.
