#ifndef MISC_UTILS_FLOATING_POINT_H
#define MISC_UTILS_FLOATING_POINT_H

#include <concepts>
#include <numbers>
#include <type_traits>

namespace micromouse
{

namespace detail
{

template <typename T>
concept PartialArithmeticHelper = requires (T a, T b) {
    { +a } noexcept -> std::convertible_to<T>;
    { ++a } noexcept -> std::same_as<T &>;
    { a++ } noexcept -> std::convertible_to<T>;
    { -a } noexcept -> std::convertible_to<T>;
    { --a } noexcept -> std::same_as<T &>;
    { a-- } noexcept -> std::convertible_to<T>;
    { a + b } noexcept -> std::convertible_to<T>;
    { a - b } noexcept -> std::convertible_to<T>;
    { a * b } noexcept -> std::convertible_to<T>;
    { a / b } noexcept -> std::convertible_to<T>;
    // { a % b } noexcept -> std::convertible_to<T>;
    { a = b } noexcept -> std::same_as<T &>;
    { a += b } noexcept -> std::same_as<T &>;
    { a -= b } noexcept -> std::same_as<T &>;
    { a *= b } noexcept -> std::same_as<T &>;
    { a /= b } noexcept -> std::same_as<T &>;
    // { a %= b } noexcept -> std::same_as<T &>;
    requires std::is_trivially_copyable_v<T>;
    requires std::is_trivially_destructible_v<T>;
    requires std::regular<T>;
    requires std::totally_ordered<T>;
    // Also require the comparisons to be noexcept
    { a <=> b } noexcept;
    { a == b } noexcept -> std::convertible_to<bool>;
    { a != b } noexcept -> std::convertible_to<bool>;
    { a < b } noexcept -> std::convertible_to<bool>;
    { a <= b } noexcept -> std::convertible_to<bool>;
    { a > b } noexcept -> std::convertible_to<bool>;
    { a >= b } noexcept -> std::convertible_to<bool>;
};

}  // namespace detail

template <typename T>
concept Arithmetic = std::integral<T> || std::floating_point<T>;

template <typename T>
concept PartialArithmetic = Arithmetic<T> || detail::PartialArithmeticHelper<std::remove_cvref_t<T>>;

template <typename T>
struct is_arithmetic_wrapper : std::false_type
{};

template <PartialArithmetic T>
    requires (Arithmetic<typename T::type>)
struct is_arithmetic_wrapper<T> : std::true_type
{};

template <PartialArithmetic T>
    requires (is_arithmetic_wrapper<typename T::type>::value)
struct is_arithmetic_wrapper<T> : std::true_type
{};

template <typename T>
inline constexpr bool is_arithmetic_wrapper_v = is_arithmetic_wrapper<T>::value;

template <typename T>
concept ArithmeticWrapper = is_arithmetic_wrapper_v<T> && PartialArithmetic<T>;

template <typename T>
concept ExtendedArithmetic = Arithmetic<T> || ArithmeticWrapper<T>;

template <typename T>
struct is_floating_point_wrapper : std::false_type
{};

template <PartialArithmetic T>
    requires (std::floating_point<typename T::type>)
struct is_floating_point_wrapper<T> : std::true_type
{};

template <PartialArithmetic T>
    requires (is_floating_point_wrapper<typename T::type>::value)
struct is_floating_point_wrapper<T> : std::true_type
{};

template <typename T>
inline constexpr bool is_floating_point_wrapper_v = is_floating_point_wrapper<T>::value;

template <typename T>
concept FloatingPointWrapper = is_floating_point_wrapper_v<T>;

template <typename T>
concept ExtendedFloatingPoint = std::floating_point<T> || FloatingPointWrapper<T>;

namespace detail
{

template <ExtendedFloatingPoint T>
struct make_floating_point_helper;

template <std::floating_point T>
struct make_floating_point_helper<T>
{
    using type = T;
};

template <FloatingPointWrapper T>
    requires std::floating_point<typename T::type>
struct make_floating_point_helper<T>
{
    using type = typename T::type;
};

template <FloatingPointWrapper T>
    requires (!std::floating_point<typename T::type>)
struct make_floating_point_helper<T> : make_floating_point_helper<typename T::type>
{};

}  // namespace detail

template <ExtendedFloatingPoint T>
using make_floating_point = typename detail::make_floating_point_helper<T>::type;

}  // namespace micromouse

#endif  // MISC_UTILS_FLOATING_POINT_H
