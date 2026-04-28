/// constexpr reloc ctor not discarded on function parameters during
/// constant evaluation. In constexpr, the reloc ctor (+100) is always
/// used, never the move ctor (+1).

#include <iostream>
#include "cxlog.h"

struct S {
    int val;
    constexpr S(int v) : val(v) {}
    constexpr S(S reloc src) : val(src.val + 100) {}
    constexpr S(S&& src) : val(src.val + 1) {}
    constexpr ~S() {}
};

constexpr int f(S s) {
    S t = reloc s;
    return t.val;
}

// reloc ctor: 42 + 100 = 142
static_assert(f(S(42)) == 142, "");

// cxlog trace in consteval: param reloc uses reloc ctor, not move.
struct SL {
    cxlog* log;
    constexpr SL(cxlog& l) : log(&l)
        { log->record('S', cxlog::CTOR, this); }
    constexpr SL(SL reloc src) : log(src.log)
        { log->record('S', cxlog::RELOC, this, &src); }
    constexpr SL(SL&& o) : log(o.log)
        { log->record('S', cxlog::MOVE, this, &o); }
    constexpr ~SL() { log->record('S', cxlog::DTOR, this); }
};

constexpr int fl(SL s) {
    cxlog& log = *s.log;
    { SL t = reloc s; (void)t; }
    return log.match_ops("S(); S(S reloc); ~S()");
}
consteval int test_log() {
    cxlog log;
    SL s(log);
    return fl(reloc s);
}
static_assert(test_log() == 1, "param reloc: reloc ctor, not move");

int main()
{
    constexpr int ct = f(S(42));
    std::cout << "compile_time: " << ct << std::endl;
    // At runtime, reloc ctor discardment may apply (caller-destroy ABI),
    // so the runtime result may differ. We only verify compile-time here.
    return 0;
}

////// BUILD SUCCESS
// compile_time: 142
