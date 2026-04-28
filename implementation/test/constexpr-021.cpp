/// constexpr event-log test: verify which ctors/dtors fire using cxlog.
/// Tests mandatory elision (local-to-local) and non-elision (parameter).

#include <iostream>
#include "cxlog.h"

// ---------------------------------------------------------------------------
// A type whose every special member records into a cxlog with addresses.
// ---------------------------------------------------------------------------
struct S {
    cxlog* log;
    char id;
    int val;

    constexpr S(cxlog& l, char c, int v) : log(&l), id(c), val(v) {
        log->record(id, cxlog::CTOR, this);
    }
    constexpr S(S reloc src) : log(src.log), id(src.id), val(src.val) {
        log->record(id, cxlog::RELOC, this, &src);
    }
    constexpr S(S&& o) : log(o.log), id(o.id), val(o.val) {
        log->record(id, cxlog::MOVE, this, &o);
    }
    constexpr S(S const& o) : log(o.log), id(o.id), val(o.val) {
        log->record(id, cxlog::COPY, this, &o);
    }
    constexpr ~S() {
        log->record(id, cxlog::DTOR, this);
    }
};

// NOTE: all tests use a nested scope so that destructors run before the
// return statement, allowing the log to capture the full event sequence.
//
// static_assert uses match_ops() which checks the event sequence without
// addresses (constexpr allocates separate storage even for elided objects).
// Runtime dump() shows normalized addresses like snoop.h for visual inspection.

// ---------------------------------------------------------------------------
// Test 1: local-to-local reloc → mandatory elision (no reloc/move/copy ctor).
// Same address at runtime proves s and t share storage.
// ---------------------------------------------------------------------------
constexpr int test_local_elision() {
    cxlog log;
    {
        S s(log, 'S', 10);
        S t = reloc s;
        (void)t;
    }
    return log.match_ops("S(); ~S()");
}
static_assert(test_local_elision() == 1, "local-to-local must be elided");

// ---------------------------------------------------------------------------
// Test 2: parameter reloc → NOT elided, ctor fires.
// At compile time (consteval): reloc ctor is called → "S() S(S reloc) ~S()"
// At runtime: reloc ctor is discarded (ABI), move ctor fires → "S() S(S&&) ~S()"
// ---------------------------------------------------------------------------
constexpr int test_param_not_elided_helper(S s) {
    cxlog& log = *s.log;
    {
        S t = reloc s;
        (void)t;
    }
    return log.match_ops("S(); S(S reloc); ~S()");
}
consteval int test_param_not_elided() {
    cxlog log;
    S s(log, 'S', 10);
    return test_param_not_elided_helper(reloc s);
}
static_assert(test_param_not_elided() == 1, "param reloc must call reloc ctor");

// ---------------------------------------------------------------------------
// Test 3: two classes A and B, interleaved.
// Both elided: A stays at 0x1, B stays at 0x2. Reverse dtor order.
// ---------------------------------------------------------------------------
constexpr int test_two_classes() {
    cxlog log;
    {
        S a(log, 'A', 1);
        S b(log, 'B', 2);
        S a2 = reloc a;
        S b2 = reloc b;
        (void)a2; (void)b2;
    }
    return log.match_ops("A(); B(); ~B(); ~A()");
}
static_assert(test_two_classes() == 1, "two classes, both elided");

// ---------------------------------------------------------------------------
// Test 4: chain of relocs: s → t → u.  All local, all elided.
// All share the same runtime address 0x1.
// ---------------------------------------------------------------------------
constexpr int test_chain() {
    cxlog log;
    {
        S s(log, 'S', 5);
        S t = reloc s;
        S u = reloc t;
        (void)u;
    }
    return log.match_ops("S(); ~S()");
}
static_assert(test_chain() == 1, "chain of relocs, all elided");

// ---------------------------------------------------------------------------
// Runtime: run each test and print the actual trace.
// ---------------------------------------------------------------------------
int main() {
    // Test 1
    {
        cxlog log;
        {
            S s(log, 'S', 10);
            S t = reloc s;
            (void)t;
        }
        std::cout << "local_elision: ";
        log.dump();
        std::cout << std::endl;
    }
    // Test 2
    {
        auto helper = [](S s) {
            cxlog& log = *s.log;
            {
                S t = reloc s;
                (void)t;
            }
            return log;
        };
        cxlog log;
        S s(log, 'S', 10);
        cxlog result = helper(reloc s);
        std::cout << "param_not_elided: ";
        result.dump();
        std::cout << std::endl;
    }
    // Test 3
    {
        cxlog log;
        {
            S a(log, 'A', 1);
            S b(log, 'B', 2);
            S a2 = reloc a;
            S b2 = reloc b;
            (void)a2; (void)b2;
        }
        std::cout << "two_classes: ";
        log.dump();
        std::cout << std::endl;
    }
    // Test 4
    {
        cxlog log;
        {
            S s(log, 'S', 5);
            S t = reloc s;
            S u = reloc t;
            (void)u;
        }
        std::cout << "chain: ";
        log.dump();
        std::cout << std::endl;
    }
    return 0;
}

////// BUILD SUCCESS
// local_elision: S() 0x1; ~S() 0x1
// param_not_elided: S() 0x1; S(S&&) 0x2 <- 0x1; ~S() 0x2
// two_classes: A() 0x1; B() 0x2; ~B() 0x2; ~A() 0x1
// chain: S() 0x1; ~S() 0x1
