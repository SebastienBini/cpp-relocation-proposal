#include "snoop.h"

#include <array>
#include <string_view>


namespace detail {

constexpr std::size_t decimalLength(int value) {
    std::size_t digits = value <= 0 ? 1 : 0;
    while (value != 0) {
        ++digits;
        value /= 10;
    }
    return digits;
}

template <int N>
constexpr auto makeIntString() {
    std::array<char, decimalLength(N) + 1> chars{};
    int value = N;
    std::size_t index = chars.size() - 1;
    chars[index] = '\0';

    if (value == 0) {
        chars[0] = '0';
        return chars;
    }

    bool negative = value < 0;
    do {
        int digit = value % 10;
        if (digit < 0)
            digit = -digit;
        chars[--index] = static_cast<char>('0' + digit);
        value /= 10;
    } while (value != 0);

    if (negative)
        chars[0] = '-';

    return chars;
}

template <int N>
struct IntStringStorage {
    static inline constexpr auto chars = makeIntString<N>();
};

} // namespace detail

template <int N>
constexpr std::string_view intToStr() {
    return {detail::IntStringStorage<N>::chars.data(),
            detail::IntStringStorage<N>::chars.size() - 1};
}

template <unsigned N, class Base> struct Intermediate : Intermediate<N-1, Base> { snoop s{intToStr<N>()}; };

template <class Base> struct Intermediate<1, Base> : Base { snoop s{"Intermediate"}; };
