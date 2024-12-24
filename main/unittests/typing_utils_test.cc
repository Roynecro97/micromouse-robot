#include "misc_utils/typing_utils.h"

#include <misc_utils/angle.h>
#include <misc_utils/strongly_typed.h>

#include "a.h"
#include "misc_utils_adapters.h"

#include <gtest/gtest.h>

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

}  // namespace micromouse::tests

REGISTER_TEST_FILE(type_utils_tests);
