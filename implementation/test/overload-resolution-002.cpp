// Test: Rule-of-Zero class must satisfy assignable_from and standard concepts
// This validates that the implicit relocation assignment operator does not
// break concept checks for xvalue arguments (the assignable_from regression).

#include <concepts>
#include <iostream>
#include <iterator>

// A simple Rule-of-Zero wrapper (like libc++'s __wrap_iter)
template <class Iter>
struct SimpleIterator {
    Iter base_;

    using value_type = typename std::iterator_traits<Iter>::value_type;
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using pointer = typename std::iterator_traits<Iter>::pointer;
    using reference = typename std::iterator_traits<Iter>::reference;
    using iterator_category = typename std::iterator_traits<Iter>::iterator_category;

    reference operator*() const { return *base_; }
    pointer operator->() const { return base_; }
    SimpleIterator& operator++() { ++base_; return *this; }
    SimpleIterator operator++(int) { auto tmp = *this; ++base_; return tmp; }
    bool operator==(const SimpleIterator&) const = default;
};

int main(int, char**)
{
    // assignable_from checks { lhs = std::forward<const T>(rhs) }
    // which produces a const T&& (xvalue) argument.
    // Without bullet 4, this is ambiguous between operator=(const T&) and operator=(T).
    static_assert(std::assignable_from<SimpleIterator<int*>&, const SimpleIterator<int*>>);
    std::cout << "assignable_from: pass" << std::endl;

    // copyable requires assignable_from
    static_assert(std::copyable<SimpleIterator<int*>>);
    std::cout << "copyable: pass" << std::endl;

    // semiregular requires copyable + default_initializable
    static_assert(std::semiregular<SimpleIterator<int*>>);
    std::cout << "semiregular: pass" << std::endl;

    std::cout << "all concepts satisfied" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// assignable_from: pass
// copyable: pass
// semiregular: pass
// all concepts satisfied
