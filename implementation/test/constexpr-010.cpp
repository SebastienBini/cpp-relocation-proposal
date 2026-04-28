/// constexpr obj.this — returns non-null pointer with identity property.

#include <iostream>

struct S {
    int x;
    constexpr S(int v) : x(v) {}
};

constexpr bool test_nonnull() {
    S reloc s(1);
    void *p = s.this;
    return p != nullptr;
}

constexpr bool test_identity() {
    S reloc s(42);
    void *p1 = s.this;
    void *p2 = s.this;
    return p1 == p2;
}

constexpr bool test_identity_2() {
    S s(42);
    void *p1 = &s;
    S reloc t = reloc s;
    void *p2 = t.this;
    return p1 == p2;
}

static_assert(test_nonnull(), "");
static_assert(test_identity(), "");
static_assert(test_identity_2(), ""); // mandatory elision preserves identity

int main()
{
    std::cout << "nonnull: " << (test_nonnull() ? "OK" : "FAIL") << std::endl;
    std::cout << "identity: " << (test_identity() ? "OK" : "FAIL") << std::endl;
    std::cout << "identity2: " << (test_identity_2() ? "OK" : "FAIL") << std::endl;

    // Runtime: verify .this returns an actual address
    S reloc s(99);
    void *p = s.this;
    std::cout << "runtime_nonnull: " << (p != nullptr ? "OK" : "FAIL") << std::endl;
    return 0;
}

////// BUILD SUCCESS
// nonnull: OK
// identity: OK
// identity2: OK
// runtime_nonnull: OK
