#ifndef MISC_UTILS_TYPING_UTILS_H
#define MISC_UTILS_TYPING_UTILS_H

#include <array>
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

/**
 * @brief A version of `std::rank` that can also handle `std::array`s.
 *
 * @tparam T The type whose rank is wanted.
 *
 * @see https://en.cppreference.com/w/cpp/types/rank
 */
template <typename T>
struct rank : std::integral_constant<std::size_t, 0>
{};

template <typename T>
struct rank<T[]> : std::integral_constant<std::size_t, rank<T>::value + 1>
{};

template <typename T, std::size_t N>
struct rank<T[N]> : std::integral_constant<std::size_t, rank<T>::value + 1>
{};

template <typename T, std::size_t N>
struct rank<std::array<T, N>> : std::integral_constant<std::size_t, rank<T>::value + 1>
{};

template <typename T>
inline constexpr auto rank_v = rank<T>::value;

/**
 * @brief A version of `std::extent` that can also handle `std::array`s.
 *
 * @tparam T The type whose extent is wanted.
 * @tparam N The index of the wanted extent.
 *
 * @see https://en.cppreference.com/w/cpp/types/extent
 */
template <typename T, std::size_t N = 0>
struct extent : std::integral_constant<std::size_t, 0>
{};

template <typename T>
struct extent<T[], 0> : std::integral_constant<std::size_t, 0>
{};

template <typename T, std::size_t E>
struct extent<T[E], 0> : std::integral_constant<std::size_t, E>
{};

template <typename T, std::size_t E>
struct extent<std::array<T, E>, 0> : std::integral_constant<std::size_t, E>
{};

template <typename T, std::size_t N>
struct extent<T[], N> : extent<T, N - 1>
{};

template <typename T, std::size_t E, std::size_t N>
struct extent<T[E], N> : extent<T, N - 1>
{};

template <typename T, std::size_t E, std::size_t N>
struct extent<std::array<T, E>, N> : extent<T, N - 1>
{};

template <typename T, std::size_t N = 0>
inline constexpr auto extent_v = extent<T, N>::value;

/**
 * @brief A version of `std::remove_extent` that can also handle `std::array`s.
 *
 * @tparam T The type whose rank is wanted.
 */
template <typename T>
struct remove_extent
{
    using type = T;
};

template <typename T>
struct remove_extent<T[]>
{
    using type = T;
};

template <typename T, std::size_t N>
struct remove_extent<T[N]>
{
    using type = T;
};

template <typename T, std::size_t N>
struct remove_extent<std::array<T, N>>
{
    using type = T;
};

template <typename T>
using remove_extent_type = remove_extent<T>::type;

/**
 * @brief A version of `std::remove_all_extents` that can also handle `std::array`s.
 *
 * @tparam T The type whose rank is wanted.
 */
template <typename T>
struct remove_all_extents
{
    using type = T;
};

template <typename T>
struct remove_all_extents<T[]> : remove_all_extents<T>
{};

template <typename T, std::size_t N>
struct remove_all_extents<T[N]> : remove_all_extents<T>
{};

template <typename T, std::size_t N>
struct remove_all_extents<std::array<T, N>> : remove_all_extents<T>
{};

template <typename T>
using remove_all_extents_type = remove_all_extents<T>::type;

namespace detail
{

template <typename T, typename ElementType, std::size_t... Extents>
struct any_array_impl : std::false_type
{};

// Special-case: with no extents, T should be the element itself
template <typename T, typename ElementType>
struct any_array_impl<T, ElementType> : std::is_same<T, ElementType>
{};

template <typename T, typename ElementType, std::size_t Extent, std::size_t... Extents>
struct any_array_impl<T, ElementType, Extent, Extents...>
    : std::conjunction<
          std::bool_constant<extent_v<T> == Extent>,
          any_array_impl<remove_extent_type<T>, ElementType, Extents...>>
{};

}  // namespace detail

template <typename T, typename ElementType, std::size_t... Extents>
concept AnyArray = sizeof...(Extents) > 0 && detail::any_array_impl<T, ElementType, Extents...>::value;

}  // namespace micromouse

#endif  // MISC_UTILS_TYPING_UTILS_H
