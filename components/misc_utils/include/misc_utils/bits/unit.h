#ifndef MISC_UTILS_BITS_UNIT_H
#define MISC_UTILS_BITS_UNIT_H

#include "physical_unit.h"
#include "unit_list.h"

#include <type_traits>

namespace micromouse
{

template <UnitListType Numerator, UnitListType Denominator = UnitList<>>
    requires (!unit_list_contains_any_v<Numerator, Denominator> && !unit_list_contains_any_v<Denominator, Numerator>)
struct Unit
{
    using num = Numerator;
    using den = Denominator;
};

template <typename T>
struct is_unit : std::false_type
{};

template <UnitListType Numerator, UnitListType Denominator>
struct is_unit<Unit<Numerator, Denominator>> : std::true_type
{};

template <typename T>
inline constexpr auto is_unit_v = is_unit<T>::value;

template <typename T>
concept UnitSpec = is_unit_v<T>;

template <PhysicalUnitType... Units>
using make_units = Unit<UnitList<Units...>>;

namespace detail
{

template <typename T>
concept UnitSpecOrSingle = UnitSpec<T> || PhysicalUnitType<T>;

template <UnitSpecOrSingle, UnitSpecOrSingle>
struct unit_mul_helper;

template <
    UnitListType LhsNumerator,
    UnitListType LhsDenominator,
    UnitListType RhsNumerator,
    UnitListType RhsDenominator>
struct unit_mul_helper<Unit<LhsNumerator, LhsDenominator>, Unit<RhsNumerator, RhsDenominator>>
{
    using raw_numerator = unit_list_add<LhsNumerator, RhsNumerator>;
    using raw_denominator = unit_list_add<LhsDenominator, RhsDenominator>;
    using numerator = typename detail::unit_list_remove_helper<raw_numerator, raw_denominator>::type;
    using denominator = typename detail::unit_list_remove_helper<raw_numerator, raw_denominator>::remainder;
    using type = Unit<numerator, denominator>;
};

template <PhysicalUnitType Lhs, PhysicalUnitType Rhs>
struct unit_mul_helper<Lhs, Rhs> : unit_mul_helper<make_units<Lhs>, make_units<Rhs>>
{};

template <PhysicalUnitType Lhs, UnitSpec Rhs>
struct unit_mul_helper<Lhs, Rhs> : unit_mul_helper<make_units<Lhs>, Rhs>
{};

template <UnitSpec Lhs, PhysicalUnitType Rhs>
struct unit_mul_helper<Lhs, Rhs> : unit_mul_helper<Lhs, make_units<Rhs>>
{};

template <UnitSpecOrSingle, UnitSpecOrSingle>
struct unit_div_helper;

template <
    UnitListType LhsNumerator,
    UnitListType LhsDenominator,
    UnitListType RhsNumerator,
    UnitListType RhsDenominator>
struct unit_div_helper<Unit<LhsNumerator, LhsDenominator>, Unit<RhsNumerator, RhsDenominator>>
    : unit_mul_helper<Unit<LhsNumerator, LhsDenominator>, Unit<RhsDenominator, RhsNumerator>>
{};

template <PhysicalUnitType Lhs, PhysicalUnitType Rhs>
struct unit_div_helper<Lhs, Rhs> : unit_div_helper<make_units<Lhs>, make_units<Rhs>>
{};

template <PhysicalUnitType Lhs, UnitSpec Rhs>
struct unit_div_helper<Lhs, Rhs> : unit_div_helper<make_units<Lhs>, Rhs>
{};

template <UnitSpec Lhs, PhysicalUnitType Rhs>
struct unit_div_helper<Lhs, Rhs> : unit_div_helper<Lhs, make_units<Rhs>>
{};

}  // namespace detail

template <detail::UnitSpecOrSingle Lhs, detail::UnitSpecOrSingle Rhs>
using unit_mul = typename detail::unit_mul_helper<Lhs, Rhs>::type;

template <detail::UnitSpecOrSingle Lhs, detail::UnitSpecOrSingle Rhs>
using unit_div = typename detail::unit_div_helper<Lhs, Rhs>::type;

template <UnitSpec T, UnitSpec U>
struct units_equal : std::is_same<unit_div<T, U>, Unit<UnitList<>>>
{};

template <UnitSpec T, UnitSpec U>
inline constexpr auto units_equal_v = units_equal<T, U>::value;

namespace detail
{
template <typename T, typename U>
concept SameUnitHelper = UnitSpec<T> && UnitSpec<U> && units_equal_v<T, U>;
}  // namespace detail

template <typename T, typename U>
concept SameUnitAs = detail::SameUnitHelper<T, U> && detail::SameUnitHelper<U, T>;

}  // namespace micromouse

#endif  // MISC_UTILS_BITS_UNIT_H
