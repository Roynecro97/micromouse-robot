#ifndef UNITTESTS_VALUES_H
#define UNITTESTS_VALUES_H

#include <concepts>
#include <numbers>

namespace micromouse::tests
{

template <ExtendedArithmetic>
struct values_helper;

template <ArithmeticWrapper T>
struct values_helper<T> : values_helper<typename T::type>
{};

template <std::signed_integral T>
struct values_helper<T>
{
    static constexpr T values[] = {-128, -100, -56, -10, -5, -2, -1, 0, 1, 2, 5, 10, 16, 78, 127};
};

template <std::unsigned_integral T>
struct values_helper<T>
{
    static constexpr T values[] = {0, 1, 2, 5, 10, 16, 56, 78, 100, 127, 141, 160, 191, 200, 226, 255};
};

template <>
struct values_helper<bool>
{
    static constexpr bool values[] = {false, true};
};

template <std::floating_point T>
struct values_helper<T>
{
    static constexpr T values[] = {
        -128,
        -100,
        -56,
        -10,
        -std::numbers::pi_v<T>,
        -std::numbers::e_v<T>,
        -2,
        -std::numbers::pi_v<T> / 2,
        -1,
        -std::numbers::pi_v<T> / 4,
        -1690e-4,
        0,
        3.56e-3,
        std::numbers::pi_v<T> / 4,
        1,
        std::numbers::pi_v<T> / 2,
        2,
        std::numbers::e_v<T>,
        std::numbers::pi_v<T>,
        5.67,
        10,
        78,
        127,
    };
};

}  // namespace micromouse::tests

#endif  // UNITTESTS_VALUES_H
