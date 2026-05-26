#include "snoop.h"
#include "box.h"

// P2785 §implicit-decomposition-ill-formed: a type with a user-provided
// destructor cannot be implicitly decomposed.  The expression remains an
// xvalue and the move constructor is used instead.

struct PairWithDtor {
    box<snoop> first;
    snoop second;
    PairWithDtor(snoop const a, snoop b) : first{reloc a}, second{reloc b} {}
    PairWithDtor(PairWithDtor reloc) = default;
    ~PairWithDtor() {
        std::cout << "~PairWithDtor()" << std::endl;
    }
};

PairWithDtor getPair() {
    PairWithDtor p{snoop{"first"}, snoop{"second"}};
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

////// BUILD FAILURE
// implicit-decomp-002-box.cpp:26:14: error: call to deleted constructor of 'box<snoop>'
//    26 |         auto s = getPair().first;
//       |              ^   ~~~~~~~~~~~~~~~
// /workspace/llvm-project/P2785/implementation/test/box.h:15:5: note: 'box' has been explicitly marked deleted here
//    15 |     box(box&&) = delete;
//       |     ^
// implicit-decomp-002-box.cpp:26:18: note: implicit decomposition of temporary of type 'PairWithDtor' is not possible because its destructor is user-provided
//    26 |         auto s = getPair().first;
//       |                  ^
// 1 error generated.
