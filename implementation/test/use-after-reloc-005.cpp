using T = int;
T getT() { return {}; }
void bar(T) {}
bool sometest(T) { return false; }
void do_smth(T) {}

void reloc_showcase_03()
{
    const T var = getT();
    if (sometest(var))
        bar(reloc var); // #1, var state is relocated
    else
        do_smth(var); // #2, var state is alive
    // var state is alive-or-relocated
}

int main() { reloc_showcase_03(); return 0; }

////// BUILD SUCCESS
