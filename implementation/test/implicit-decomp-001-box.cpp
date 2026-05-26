#include "snoop.h"
#include "box.h"

// P2785 §implicit-decomposition: data member access on a temporary.
// When getPair().first is used to copy-initialize a variable, the temporary
// pair is implicitly decomposed: .first is relocated (not moved), and .second
// is destroyed individually (the complete-object destructor is NOT called).

struct Pair {
    box<snoop> first;
    snoop second;
};

Pair getPair() {
    Pair p{snoop{"first"}, snoop{"second"}};
    std::cout << "--- p is constructed" << std::endl;
    return p;
}

int main(int, char**) {
    std::cout << "---" << std::endl;
    {
        auto s = getPair().first;
        std::cout << "--- " << s->name << " alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---
// first() 0x1
// first(first&&) 0x2 <- 0x1
// second() 0x3
// ~first() 0x1
// --- p is constructed
// ~second() 0x3
// --- first alive ---
// ~first() 0x2
// ---
