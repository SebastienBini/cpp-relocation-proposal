using T = int;
T getT() { return {}; }
void do_smth(T) {}

void reloc_showcase_11()
{
    const T var = getT();
    for (int i = 0; i != 10; ++i)
    {
        if (i == 9) {
            do_smth(reloc var); // OK, var state is relocated
            break;
        }
        else
            do_smth(var); // OK, var state is alive
    }
    // var state is A-R
}

int main() { reloc_showcase_11(); return 0; }

////// BUILD SUCCESS
