// Test: overload resolution [over.ics.rank]/3.2.3 with -frelocation
// Validates that for each value category, the correct overload is selected
// when both reference-binding and by-value candidates are viable.

#include <iostream>

struct T {
    int val;
};

// --- Overload set 1: const T& vs T vs T&& (all three) ---

std::string_view f1(const T&) { return "f1(const T&)"; }
std::string_view f1(T)        { return "f1(T)"; }
std::string_view f1(T&&)      { return "f1(T&&)"; }

// --- Overload set 2: const T& vs T (no rvalue ref) ---
// This is the critical set: xvalue must prefer const T& over T.

std::string_view f2(const T&) { return "f2(const T&)"; }
std::string_view f2(T)        { return "f2(T)"; }

// --- Overload set 3: T vs T&& (no lvalue ref) ---

std::string_view f3(T)   { return "f3(T)"; }
std::string_view f3(T&&) { return "f3(T&&)"; }

// --- Overload set 4: T vs T&& (no lvalue ref) ---

std::string_view f4(T const&)   { return "f4(T const&)"; }
std::string_view f4(T&&) { return "f4(T&&)"; }

T make_t() { return T{42}; }

int main(int, char**)
{
    T obj{42};

    std::cout << "Overload set 1: all three overloads" << std::endl;
    std::cout << "f1(obj)           = " << f1(obj) << std::endl;            // lvalue -> const T&
    std::cout << "f1(make_t())      = " << f1(make_t()) << std::endl;       // prvalue -> T
    std::cout << "f1(std::move(obj))= " << f1(std::move(obj)) << std::endl; // xvalue -> T&&
    std::cout << std::endl;
    std::cout << "Overload set 2: const T& vs T — the new bullet 4" << std::endl;
    std::cout << "f2(obj)           = " << f2(obj) << std::endl;            // lvalue -> const T&
    std::cout << "f2(make_t())      = " << f2(make_t()) << std::endl;       // prvalue -> T
    std::cout << "f2(std::move(obj))= " << f2(std::move(obj)) << std::endl; // xvalue -> const T& (NEW!)
    std::cout << std::endl;
    std::cout << "Overload set 3: T vs T&&" << std::endl;
    std::cout << "f3(obj)           = " << f3(obj) << std::endl;            // lvalue -> T
    std::cout << "f3(make_t())      = " << f3(make_t()) << std::endl;       // prvalue -> T
    std::cout << "f3(std::move(obj))= " << f3(std::move(obj)) << std::endl; // xvalue -> T&&
    std::cout << std::endl;
    std::cout << "Overload set 4: T const& vs T&&" << std::endl;
    std::cout << "f4(obj)           = " << f4(obj) << std::endl;            // xvalue -> T const&
    std::cout << "f4(make_t())      = " << f4(make_t()) << std::endl;       // prvalue -> T&&
    std::cout << "f4(std::move(obj))= " << f4(std::move(obj)) << std::endl; // xvalue -> T&&

    return 0;
}

////// BUILD SUCCESS
// Overload set 1: all three overloads
// f1(obj)           = f1(const T&)
// f1(make_t())      = f1(T)
// f1(std::move(obj))= f1(T&&)
//
// Overload set 2: const T& vs T — the new bullet 4
// f2(obj)           = f2(const T&)
// f2(make_t())      = f2(T)
// f2(std::move(obj))= f2(const T&)
//
// Overload set 3: T vs T&&
// f3(obj)           = f3(T)
// f3(make_t())      = f3(T)
// f3(std::move(obj))= f3(T&&)
//
// Overload set 4: T const& vs T&&
// f4(obj)           = f4(T const&)
// f4(make_t())      = f4(T&&)
// f4(std::move(obj))= f4(T&&)
