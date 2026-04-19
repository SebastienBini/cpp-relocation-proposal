using T = int;
T getT() { return {}; }
void bar(T) {}
bool sometest(T) { return false; }
void do_smth(T) {}
void do_smth_else(T) {}

template<typename U>
struct my_can_relocate {
    constexpr bool operator()() const { return true; }
};

void reloc_showcase_07()
{
    constexpr bool relocated_v = my_can_relocate<T>{}();
    const T var = getT();
    if constexpr(relocated_v)
    {
        bar(reloc var);
    }
    else
        do_smth(var); // OK
    // [...]
    // var state is R if relocated_v is true, A otherwise
    if constexpr(!relocated_v)
        do_smth_else(var); // OK
}

int main() { reloc_showcase_07(); return 0; }

////// BUILD SUCCESS
