#include "snoop.h"

// P2785 §implicit-decomposition-ill-formed: a type with a user-provided
// destructor cannot be implicitly decomposed.  The expression remains an
// xvalue and the move constructor is used instead.

struct PairWithDtor {
    snoop first;
    snoop second;
    ~PairWithDtor() {
        std::cout << "~PairWithDtor()" << std::endl;
    }
};

PairWithDtor getPair() {
    PairWithDtor p{snoop{"first"}, snoop{"second"}};
    std::cout << "--- p is constructed" << std::endl;
    return p;
}

int main(int, char**) {
    std::cout << "---" << std::endl;
    {
        snoop s = getPair().first;
        std::cout << "--- s alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---
// first() 0x1
// second() 0x2
// --- p is constructed
// first(first&&) 0x3 <- 0x1
// ~PairWithDtor()
// ~second() 0x2
// ~first() 0x1
// --- s alive ---
// ~first() 0x3
// ---
