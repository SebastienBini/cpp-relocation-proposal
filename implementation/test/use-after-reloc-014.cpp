using T = int;
T getT() { return {}; }
void do_smth(T) {}

void reloc_showcase_12()
{
    for (int i = 0; i != 10; ++i)
    {
        const T var = getT();
        do_smth(reloc var); // OK
    }
}

int main() { reloc_showcase_12(); return 0; }

////// BUILD SUCCESS
