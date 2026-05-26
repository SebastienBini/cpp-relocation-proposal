#include "snoop.h"
#include "box.h"

// P2785 §implicit-decomposition-ill-formed: under the new rules, a type with
// a private member CAN be implicitly decomposed as long as all subobjects have
// an accessible destructor.  snoop has a public destructor, so implicit
// decomposition is well-formed here.  box<snoop> is relocated via trivial
// relocation (bitwise copy).

class PrivateMemberPair {
public:
    box<snoop> pub;
private:
    snoop priv;
public:
    PrivateMemberPair(snoop a, snoop b) : pub(reloc a), priv(reloc b) {}
};

PrivateMemberPair getPair() {
    PrivateMemberPair p{snoop{"pub"}, snoop{"priv"}};
    std::cout << "--- p is constructed" << std::endl;
    return p;
}

int main(int, char**) {
    std::cout << "---" << std::endl;
    {
        auto s = getPair().pub;
        std::cout << "--- " << s->name << " alive ---" << std::endl;
    }
    std::cout << "---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---
// pub() 0x1
// priv() 0x2
// pub(pub&&) 0x3 <- 0x1
// pub(pub&&) 0x4 <- 0x3
// ~pub() 0x3
// priv(priv&&) 0x5 <- 0x2
// ~priv() 0x2
// ~pub() 0x1
// --- p is constructed
// ~priv() 0x5
// --- pub alive ---
// ~pub() 0x4
// ---
