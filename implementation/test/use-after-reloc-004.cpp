using T = int;
T getT();
void bar(T);
bool sometest(T);
void do_smth(T);

void reloc_showcase_02()
{
    const T var{};
    {
        const T var{};
        bar(reloc var);
        do_smth(var); // #1 ill-formed: state of 'var' is relocated
        {
            const T var{};
            do_smth(var); // #2 OK
        }
        do_smth(var); // #3 ill-formed: state of 'var' is relocated
    }
    do_smth(var); // #4 OK
}

////// BUILD FAILURE
// use-after-reloc-004.cpp:13:17: error: use of 'var' after it has been relocated
//    13 |         do_smth(var); // #1 ill-formed: state of 'var' is relocated
//       |                 ^
// use-after-reloc-004.cpp:12:13: note: relocated here
//    12 |         bar(reloc var);
//       |             ^
// use-after-reloc-004.cpp:18:17: error: use of 'var' after it has been relocated
//    18 |         do_smth(var); // #3 ill-formed: state of 'var' is relocated
//       |                 ^
// use-after-reloc-004.cpp:12:13: note: relocated here
//    12 |         bar(reloc var);
//       |             ^
// 2 errors generated.
