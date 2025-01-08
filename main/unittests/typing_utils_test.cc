#include "misc_utils/typing_utils.h"

#include <misc_utils/angle.h>
#include <misc_utils/strongly_typed.h>

#include "a.h"
#include "misc_utils_adapters.h"

#include <gtest/gtest.h>

#include <array>

#include <hack.h>

namespace micromouse::tests
{

using StrongInt = StronglyTyped<int>;
using StrongBool = StronglyTyped<bool>;
using StrongA = StronglyTyped<A>;

static_assert(detail::PartialArithmeticHelper<A>);
static_assert(detail::PartialArithmeticHelper<StrongInt>);
static_assert(std::is_same_v<decltype(!std::declval<StrongBool>()), StrongBool>);
static_assert(std::is_same_v<decltype(!std::declval<StrongInt>()), bool>);
static_assert(detail::PartialArithmeticHelper<StrongA>);
static_assert(detail::PartialArithmeticHelper<char>);
static_assert(detail::PartialArithmeticHelper<signed char>);
static_assert(detail::PartialArithmeticHelper<short>);
static_assert(detail::PartialArithmeticHelper<int>);
static_assert(detail::PartialArithmeticHelper<long>);
static_assert(detail::PartialArithmeticHelper<long long>);
static_assert(detail::PartialArithmeticHelper<unsigned char>);
static_assert(detail::PartialArithmeticHelper<unsigned short>);
static_assert(detail::PartialArithmeticHelper<unsigned int>);
static_assert(detail::PartialArithmeticHelper<unsigned long>);
static_assert(detail::PartialArithmeticHelper<unsigned long long>);
static_assert(detail::PartialArithmeticHelper<float>);
static_assert(detail::PartialArithmeticHelper<double>);
static_assert(detail::PartialArithmeticHelper<long double>);
static_assert(PartialArithmetic<A>);
static_assert(PartialArithmetic<StrongInt>);
static_assert(PartialArithmetic<StrongA>);
static_assert(PartialArithmetic<char>);
static_assert(PartialArithmetic<signed char>);
static_assert(PartialArithmetic<short>);
static_assert(PartialArithmetic<int>);
static_assert(PartialArithmetic<long>);
static_assert(PartialArithmetic<long long>);
static_assert(PartialArithmetic<unsigned char>);
static_assert(PartialArithmetic<unsigned short>);
static_assert(PartialArithmetic<unsigned int>);
static_assert(PartialArithmetic<unsigned long>);
static_assert(PartialArithmetic<unsigned long long>);
static_assert(PartialArithmetic<float>);
static_assert(PartialArithmetic<double>);
static_assert(PartialArithmetic<long double>);

struct B
{
    B operator+() const;
    B &operator++();
    B operator++(int) const;
    B operator-() const;
    B &operator--();
    B operator--(int) const;
    friend B operator+(const B &, const B &);
    friend B operator-(const B &, const B &);
    friend B operator*(const B &, const B &);
    friend B operator/(const B &, const B &);
    B &operator+=(const B &);
    B &operator-=(const B &);
    B &operator*=(const B &);
    B &operator/=(const B &);
    friend auto operator<=>(const B &, const B &) = default;
    friend bool operator==(const B &, const B &) = default;
};

static_assert(!PartialArithmetic<const char *>);
static_assert(!PartialArithmetic<B>, "B isn't a PartialArithmetic because it has throwing operators");

static_assert(std::is_same_v<make_floating_point<float>, float>);
static_assert(std::is_same_v<make_floating_point<double>, double>);
static_assert(std::is_same_v<make_floating_point<long double>, long double>);
static_assert(std::is_same_v<make_floating_point<Angle>, Angle::type>);
static_assert(std::is_same_v<make_floating_point<StronglyTyped<Angle>>, Angle::type>);

template <typename T>
struct test_rank : std::bool_constant<micromouse::rank_v<T> == std::rank_v<T>>
{};

static_assert(test_rank<int>::value);
static_assert(test_rank<int[]>::value);
static_assert(test_rank<int[11]>::value);
static_assert(test_rank<int[][22]>::value);
static_assert(test_rank<int[11][22]>::value);
static_assert(test_rank<int[][22][33]>::value);
static_assert(test_rank<int[11][22][33]>::value);
static_assert(micromouse::rank_v<std::array<int, 11>> == 1);
static_assert(micromouse::rank_v<std::array<int[22], 11>> == 2);
static_assert(micromouse::rank_v<std::array<int, 22>[]> == 2);
static_assert(micromouse::rank_v<std::array<int, 22>[11]> == 2);
static_assert(micromouse::rank_v<std::array<std::array<int, 22>, 11>> == 2);
static_assert(micromouse::rank_v<std::array<int[22][33], 11>> == 3);
static_assert(micromouse::rank_v<std::array<int[33], 22>[]> == 3);
static_assert(micromouse::rank_v<std::array<int[33], 22>[11]> == 3);
static_assert(micromouse::rank_v<std::array<std::array<int[33], 22>, 11>> == 3);
static_assert(micromouse::rank_v<std::array<int, 33>[][22]> == 3);
static_assert(micromouse::rank_v<std::array<int, 33>[11][22]> == 3);
static_assert(micromouse::rank_v<std::array<std::array<int, 33>[22], 11>> == 3);
static_assert(micromouse::rank_v<std::array<std::array<int, 33>, 22>[]> == 3);
static_assert(micromouse::rank_v<std::array<std::array<int, 33>, 22>[11]> == 3);
static_assert(micromouse::rank_v<std::array<std::array<std::array<int, 33>, 22>, 11>> == 3);

template <typename T, std::size_t N>
struct remove_extents : remove_extents<std::remove_extent_t<T>, N - 1>
{};

template <typename T>
struct remove_extents<T, 0>
{
    using type = T;
};

template <typename T, std::size_t N = 0>
using test_extent = std::bool_constant<micromouse::extent_v<T, N> == std::extent_v<T, N>>;

template <typename T, std::size_t... Ns>
using test_extents = std::conjunction<test_extent<T, Ns>...>;

template <typename T, std::size_t N = 0>
using test_manual_extent = std::conditional_t<
    micromouse::rank_v<T> <= N || std::is_unbounded_array_v<typename remove_extents<T, N>::type>,
    std::bool_constant<micromouse::extent_v<T, N> == 0>,
    std::bool_constant<micromouse::extent_v<T, N> == (N + 1) * 11>>;

template <typename T, std::size_t... Ns>
using test_manual_extents = std::conjunction<test_manual_extent<T, Ns>...>;

static_assert(test_extents<int, 0, 1, 2>::value);
static_assert(test_extents<int[], 0, 1, 2>::value);
static_assert(test_extents<int[11], 0, 1, 2>::value);
static_assert(test_extents<int[][22], 0, 1, 2>::value);
static_assert(test_extents<int[11][22], 0, 1, 2>::value);
static_assert(test_extents<int[][22][33], 0, 1, 2>::value);
static_assert(test_extents<int[11][22][33], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int, 11>, 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int, 11>, 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int[22], 11>, 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int, 22>[], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int, 22>[11], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<std::array<int, 22>, 11>, 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int[22][33], 11>, 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int[33], 22>[], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int[33], 22>[11], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<std::array<int[33], 22>, 11>, 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int, 33>[][22], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<int, 33>[11][22], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<std::array<int, 33>[22], 11>, 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<std::array<int, 33>, 22>[], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<std::array<int, 33>, 22>[11], 0, 1, 2>::value);
static_assert(test_manual_extents<std::array<std::array<std::array<int, 33>, 22>, 11>, 0, 1, 2>::value);

template <typename T, typename E>
using test_manual_remove_extent = std::is_same<micromouse::remove_extent_type<T>, E>;

template <typename T>
using test_remove_extent = test_manual_remove_extent<T, std::remove_extent_t<T>>;

static_assert(test_remove_extent<int>::value);
static_assert(test_remove_extent<int[]>::value);
static_assert(test_remove_extent<int[11]>::value);
static_assert(test_manual_remove_extent<std::array<int, 11>, int>::value);
static_assert(test_remove_extent<int[][22]>::value);
static_assert(test_remove_extent<int[11][22]>::value);
static_assert(test_manual_remove_extent<std::array<int[22], 11>, int[22]>::value);
static_assert(test_remove_extent<std::array<int, 22>[]>::value);
static_assert(test_remove_extent<std::array<int, 22>[11]>::value);
static_assert(test_manual_remove_extent<std::array<std::array<int, 22>, 11>, std::array<int, 22>>::value);
static_assert(test_remove_extent<int[][22][33]>::value);
static_assert(test_remove_extent<int[11][22][33]>::value);
static_assert(test_manual_remove_extent<std::array<int[22][33], 11>, int[22][33]>::value);
static_assert(test_remove_extent<std::array<int[33], 22>[]>::value);
static_assert(test_remove_extent<std::array<int[33], 22>[11]>::value);
static_assert(test_manual_remove_extent<std::array<std::array<int[33], 22>, 11>, std::array<int[33], 22>>::value);
static_assert(test_remove_extent<std::array<int, 33>[][22]>::value);
static_assert(test_remove_extent<std::array<int, 33>[11][22]>::value);
static_assert(test_manual_remove_extent<std::array<std::array<int, 33>[22], 11>, std::array<int, 33>[22]>::value);
static_assert(test_remove_extent<std::array<std::array<int, 33>, 22>[]>::value);
static_assert(test_remove_extent<std::array<std::array<int, 33>, 22>[11]>::value);
static_assert(test_manual_remove_extent<
              std::array<std::array<std::array<int, 33>, 22>, 11>,
              std::array<std::array<int, 33>, 22>>::value);

template <typename T, typename E = int>
using test_remove_all_extents = std::is_same<micromouse::remove_all_extents_type<T>, E>;

static_assert(test_remove_all_extents<int>::value);
static_assert(test_remove_all_extents<int[]>::value);
static_assert(test_remove_all_extents<int[11]>::value);
static_assert(test_remove_all_extents<std::array<int, 11>>::value);
static_assert(test_remove_all_extents<int[][22]>::value);
static_assert(test_remove_all_extents<int[11][22]>::value);
static_assert(test_remove_all_extents<std::array<int[22], 11>>::value);
static_assert(test_remove_all_extents<std::array<int, 22>[]>::value);
static_assert(test_remove_all_extents<std::array<int, 22>[11]>::value);
static_assert(test_remove_all_extents<std::array<std::array<int, 22>, 11>>::value);
static_assert(test_remove_all_extents<int[][22][33]>::value);
static_assert(test_remove_all_extents<int[11][22][33]>::value);
static_assert(test_remove_all_extents<std::array<int[22][33], 11>>::value);
static_assert(test_remove_all_extents<std::array<int[33], 22>[]>::value);
static_assert(test_remove_all_extents<std::array<int[33], 22>[11]>::value);
static_assert(test_remove_all_extents<std::array<std::array<int[33], 22>, 11>>::value);
static_assert(test_remove_all_extents<std::array<int, 33>[][22]>::value);
static_assert(test_remove_all_extents<std::array<int, 33>[11][22]>::value);
static_assert(test_remove_all_extents<std::array<std::array<int, 33>[22], 11>>::value);
static_assert(test_remove_all_extents<std::array<std::array<int, 33>, 22>[]>::value);
static_assert(test_remove_all_extents<std::array<std::array<int, 33>, 22>[11]>::value);
static_assert(test_remove_all_extents<std::array<std::array<std::array<int, 33>, 22>, 11>>::value);

static_assert(!AnyArray<int, int>);
static_assert(AnyArray<int[], int, 0>);
static_assert(AnyArray<int[11], int, 11>);
static_assert(AnyArray<std::array<int, 11>, int, 11>);
static_assert(AnyArray<int[][22], int, 0, 22>);
static_assert(AnyArray<int[11][22], int, 11, 22>);
static_assert(AnyArray<std::array<int[22], 11>, int, 11, 22>);
static_assert(AnyArray<std::array<int, 22>[], int, 0, 22>);
static_assert(AnyArray<std::array<int, 22>[11], int, 11, 22>);
static_assert(AnyArray<std::array<std::array<int, 22>, 11>, int, 11, 22>);
static_assert(AnyArray<int[][22][33], int, 0, 22, 33>);
static_assert(AnyArray<int[11][22][33], int, 11, 22, 33>);
static_assert(AnyArray<std::array<int[22][33], 11>, int, 11, 22, 33>);
static_assert(AnyArray<std::array<int[33], 22>[], int, 0, 22, 33>);
static_assert(AnyArray<std::array<int[33], 22>[11], int, 11, 22, 33>);
static_assert(AnyArray<std::array<std::array<int[33], 22>, 11>, int, 11, 22, 33>);
static_assert(AnyArray<std::array<int, 33>[][22], int, 0, 22, 33>);
static_assert(AnyArray<std::array<int, 33>[11][22], int, 11, 22, 33>);
static_assert(AnyArray<std::array<std::array<int, 33>[22], 11>, int, 11, 22, 33>);
static_assert(AnyArray<std::array<std::array<int, 33>, 22>[], int, 0, 22, 33>);
static_assert(AnyArray<std::array<std::array<int, 33>, 22>[11], int, 11, 22, 33>);
static_assert(AnyArray<std::array<std::array<std::array<int, 33>, 22>, 11>, int, 11, 22, 33>);

}  // namespace micromouse::tests

REGISTER_TEST_FILE(type_utils_tests);
