using T = int;
T getT() { return {}; }
bool sometest(T) { return false; }
void do_smth(T) {}

void reloc_showcase_14()
{
    const T var = getT();
from:
    if (sometest(var)) // var state is alive
    {
        do_smth(var); // var state is alive
        goto from;
    }
    else
    {
        do_smth(reloc var);
    }
}

int main() { reloc_showcase_14(); return 0; }

////// BUILD SUCCESS
