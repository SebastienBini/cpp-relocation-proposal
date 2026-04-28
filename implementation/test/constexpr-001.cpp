/// constexpr reloc on scalar types (int, pointer, enum, double, bool).
/// Verifies values are correctly transferred at both compile time and runtime.

#include <iostream>

enum Color { Red, Green, Blue };

constexpr int reloc_int() {
    int x = 42;
    return reloc x;
}

static int g = 5;
constexpr int const* reloc_ptr() {
    int const* p = &g;
    return reloc p;
}

constexpr Color reloc_enum() {
    Color c = Green;
    return reloc c;
}

constexpr double reloc_double() {
    double d = 3.14;
    return reloc d;
}

constexpr bool reloc_bool() {
    bool b = true;
    return reloc b;
}

static_assert(reloc_int() == 42, "");
static_assert(reloc_ptr() == &g, "");
static_assert(reloc_enum() == Green, "");
static_assert(reloc_double() == 3.14, "");
static_assert(reloc_bool(), "");

int main()
{
    std::cout << "reloc_int: " << reloc_int() << std::endl;
    std::cout << "reloc_ptr: " << (reloc_ptr() == &g ? "OK" : "FAIL") << std::endl;
    std::cout << "reloc_enum: " << reloc_enum() << std::endl;
    std::cout << "reloc_double: " << reloc_double() << std::endl;
    std::cout << "reloc_bool: " << reloc_bool() << std::endl;
    return 0;
}

////// BUILD SUCCESS
// reloc_int: 42
// reloc_ptr: OK
// reloc_enum: 1
// reloc_double: 3.14
// reloc_bool: 1
