int main(int argc, char**)
{
    int a = 0;
    if (argc & 1)
    {
        int b = reloc a;
    }
    return a;
}

////// BUILD FAILURE
// use-after-reloc-002.cpp:8:12: error: use of 'a' after it may have been relocated
//     8 |     return a;
//       |            ^
// 1 error generated.
