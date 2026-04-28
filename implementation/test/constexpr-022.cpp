/// constexpr chained reloc with cxlog: local → function param → reloc param.
///
/// Like constexpr-015 but with cxlog to verify which ctors/dtors actually fire.
///   - reloc a → identity(S s):  mandatory elision (local-to-param), no ctor
///   - return reloc s:           param reloc, reloc ctor IS called
///   - return prvalue → b:       C++17 guaranteed copy elision, no ctor
///   - param s consumed by reloc: no dtor for s

#include "cxlog.h"
#include <iostream>

struct S {
    cxlog* log; char id; int val;
    constexpr S(cxlog& l, char c, int v) : log(&l), id(c), val(v)
        { log->record(id, cxlog::CTOR, this); }
    constexpr S(S reloc src) : log(src.log), id(src.id), val(src.val)
        { log->record(id, cxlog::RELOC, this, &src); }
    constexpr ~S() { log->record(id, cxlog::DTOR, this); }
};

constexpr S identity(S s) { return reloc s; }

// -----------------------------------------------------------------------
// test_chain_ops: local → param elided, param reloc fires, param consumed.
// Expected ops: S()           — construct a (elided into param s)
//               S(S reloc)    — reloc ctor from param s into return slot (= b)
//               ~S()          — destroy b at end of scope
// -----------------------------------------------------------------------
constexpr int test_chain_ops() {
    cxlog log;
    {
        S a(log, 'S', 77);
        S b = identity(reloc a);
        (void)b;
    }
    return log.match_ops("S(); S(S reloc); ~S()");
}
static_assert(test_chain_ops() == 1, "ops: local->param elided, param reloc fires");

// -----------------------------------------------------------------------
// test_chain_value: the value survives the whole chain.
// -----------------------------------------------------------------------
constexpr int test_chain_value() {
    cxlog log;
    S a(log, 'S', 77);
    S b = identity(reloc a);
    return b.val;
}
static_assert(test_chain_value() == 77, "value preserved through chain");

// -----------------------------------------------------------------------
// test_chain_addrs: check address relationships via match().
// At runtime, the local 'a' is elided into param 's' (same address 0x1).
// reloc s inside identity creates a new object at 0x2 (the return slot / b).
// NOTE: In constexpr, local and param live in different frames, so their
// addresses differ. We only check addresses at runtime.
// -----------------------------------------------------------------------
int test_chain_addrs() {
    cxlog log;
    {
        S a(log, 'S', 77);
        S b = identity(reloc a);
        (void)b;
    }
    return log.match("S() 0x1; S(S reloc) 0x2 <- 0x1; ~S() 0x2");
}

int main() {
    // Compile-time results
    constexpr int ops_ct  = test_chain_ops();
    constexpr int val_ct  = test_chain_value();
    std::cout << "ops_ct: "  << (ops_ct  ? "OK" : "FAIL") << std::endl;
    std::cout << "val_ct: "  << val_ct << std::endl;

    // Runtime results
    int ops_rt  = test_chain_ops();
    int val_rt  = test_chain_value();
    int addr_rt = test_chain_addrs();
    std::cout << "ops_rt: "  << (ops_rt  ? "OK" : "FAIL") << std::endl;
    std::cout << "val_rt: "  << val_rt << std::endl;
    std::cout << "addr_rt: " << (addr_rt ? "OK" : "FAIL") << std::endl;

    // Dump a runtime trace for manual inspection
    std::cout << "trace: ";
    {
        cxlog log;
        {
            S a(log, 'S', 77);
            S b = identity(reloc a);
            (void)b;
        }
        log.dump();
    }
    std::cout << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ops_ct: OK
// val_ct: 77
// ops_rt: OK
// val_rt: 77
// addr_rt: OK
// trace: S() 0x1; S(S reloc) 0x2 <- 0x1; ~S() 0x2
