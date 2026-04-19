using T = int;
using U = float;
T getT() { return {}; }
U getU() { return {}; }
bool sometest(T) { return false; }
void do_smth(T) {}
void do_smth_else(U) {}

void reloc_showcase_15()
{
    union { T x; U y; };
    x = getT();
    if (sometest(x))
    {
        do_smth(reloc x);
        x = getT(); // well-formed
    }
    else
    {
        reloc x; // destroy x
        y = getU();
        do_smth_else(reloc y); // well-formed
        x = getT(); // well-formed
    }
    do_smth_else(reloc y); // well-formed but UB as y is not the active member on this path
}

int main() { reloc_showcase_15(); return 0; }

////// BUILD SUCCESS
