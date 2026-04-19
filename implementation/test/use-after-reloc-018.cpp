struct T { int m; };
T getT() { return {}; }
bool sometest(T) { return false; }
void do_smth(T) {}
void do_smth_else(int) {}
void do_another_thing(T) {}

void reloc_showcase_16()
{
    T x;
    if (sometest(x))
    {
        do_smth(reloc x);
    }
    else
    {
        auto const reloc y = reloc x;
        do_smth_else(reloc y.m); // well-formed
        // equivalent to: do_smth_else((reloc x).m);
    }
    do_another_thing(x); // ill-formed: state is now R
}

////// BUILD FAILURE
// use-after-reloc-018.cpp:17:20: error: decomposed object must have a non-union class type; 'const auto' is not a class type
//    17 |         auto const reloc y = reloc x;
//       |                    ^
// 1 error generated.
