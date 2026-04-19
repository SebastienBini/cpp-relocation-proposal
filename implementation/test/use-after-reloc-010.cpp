using T = int;
T getT() { return {}; }
void bar(T) {}
bool sometest(T) { return false; }
void do_smth(T) {}

void reloc_showcase_08()
{
    const T var = getT();
    if (sometest(var))
    {
        bar(reloc var);
        return;
    }
    do_smth(var); // OK
}

int main() { reloc_showcase_08(); return 0; }

////// BUILD SUCCESS
