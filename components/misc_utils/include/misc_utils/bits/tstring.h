#ifndef MISC_UTILS_BITS_TSTRING_H
#define MISC_UTILS_BITS_TSTRING_H

#include <algorithm>
#include <cstddef>
#include <iterator>

namespace micromouse
{

/**
 * @brief A literal type for strings as non-type template parameters.
 *
 * @example template <TString S>
 *          consteval auto operator""_str() noexcept
 *          { return S; }
 *
 * @tparam N    Max size, don't specify when using as a non-type template parameter's type.
 */
template <std::size_t N>
struct TString
{
    consteval TString() noexcept = default;
    consteval TString(const char (&s)[N]) noexcept { std::copy(std::begin(s), std::end(s), value); }

    constexpr operator const char *() const noexcept { return value; }

    char value[N]{};
};

}  // namespace micromouse

#endif  // MISC_UTILS_BITS_TSTRING_H
