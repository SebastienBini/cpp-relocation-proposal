int main(int, char**)
{
    int a = 0;
    int b = reloc a;
    return a;
}

////// BUILD FAILURE
// use-after-reloc-001.cpp:5:12: error: use of 'a' after it has been relocated
//     5 |     return a;
//       |            ^
// use-after-reloc-001.cpp:4:13: note: relocated here
//     4 |     int b = reloc a;
//       |             ^
// 1 error generated.
