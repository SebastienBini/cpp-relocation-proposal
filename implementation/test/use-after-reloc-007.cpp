using T = int;
T getT() { return {}; }
void bar(T) {}
bool sometest(T) { return false; }
void do_smth(T) {}

void reloc_showcase_05()
{
    const T var = getT();
    if (sometest(var))
        bar(reloc var);
    else
        do_smth(reloc var); // OK
    // var state is relocated
}

int main() { reloc_showcase_05(); return 0; }

////// BUILD SUCCESS
