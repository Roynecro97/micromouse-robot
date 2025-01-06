#ifndef BITS_TO_STRING_H
#define BITS_TO_STRING_H

#include "tstring.h"

#include <array>

namespace micromouse::detail
{

inline constexpr std::array digits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

template <std::unsigned_integral U, U base = 10>
    requires (base > 1 && base <= digits.size())
consteval auto to_string(U val) noexcept
{
    TString<std::numeric_limits<U>::digits10 + 2> res{};
    auto pos = std::end(res.value) - 1;
    if (val == 0)
    {
        *--pos = digits[0];
    }
    else
    {
        while (val > 0)
        {
            *--pos = digits[val % base];
            val /= base;
        }
    }
    std::copy(pos, std::end(res.value), std::begin(res.value));
    return res;
}

template <std::signed_integral S, S base = 10>
    requires (base > 1 && base <= digits.size())
consteval auto to_string(S val) noexcept
{
    TString<std::numeric_limits<S>::digits10 + 3> res{};
    auto pos = std::end(res.value) - 1;
    if (val == 0)
    {
        *--pos = digits[0];
    }
    else
    {
        bool neg = val < 0;
        if (!neg)
        {
            val = -val;
        }
        while (val != 0)
        {
            *--pos = digits[-(val % base)];
            val /= base;
        }
        if (neg)
        {
            *--pos = '-';
        }
    }
    std::copy(pos, std::end(res.value), std::begin(res.value));
    return res;
}

}  // namespace micromouse::detail

#endif  // BITS_TO_STRING_H
