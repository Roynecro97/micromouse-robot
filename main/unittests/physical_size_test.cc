#include "misc_utils/physical_size.h"

#include <misc_utils/unit_symbols.h>

#include "misc_utils_adapters.h"
#include "type_list.h"
#include "values.h"

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include <hack.h>

namespace micromouse::tests
{

using meters = PhysicalSize<std::int64_t, make_units<Distance>>;
using centimeters = PhysicalSize<std::int64_t, make_units<Distance>, std::centi>;
using millimeters = PhysicalSize<std::int64_t, make_units<Distance>, std::milli>;
using meters_per_second = PhysicalSize<std::int64_t, unit_div<Distance, Time>>;

consteval auto operator""_m(unsigned long long val)
{
    return meters(val);
}

consteval auto operator""_cm(unsigned long long val)
{
    return centimeters(val);
}

consteval auto operator""_mm(unsigned long long val)
{
    return millimeters(val);
}

consteval auto operator""_mps(unsigned long long val)
{
    return meters_per_second(val);
}

using Mass = PhysicalUnit<'g'>;
using Temperature = PhysicalUnit<'K'>;

using namespace std::chrono_literals;
using namespace std::string_view_literals;

// Concepts
static_assert(PhysicalUnitType<Distance>);
static_assert(PhysicalUnitType<Time>);
static_assert(PhysicalUnitType<Mass>);
static_assert(PhysicalUnitType<Temperature>);
static_assert(!PhysicalUnitType<int>);
static_assert(!PhysicalUnitType<UnitList<Distance>>);
static_assert(!PhysicalUnitType<UnitList<Distance, Time>>);
static_assert(!PhysicalUnitType<Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(!PhysicalUnitType<std::ratio<1>>);
static_assert(!PhysicalUnitType<std::milli>);
static_assert(!PhysicalUnitType<std::mega>);
static_assert(!PhysicalUnitType<meters>);
static_assert(!PhysicalUnitType<::micromouse::millimeters>);

static_assert(!UnitListType<Distance>);
static_assert(!UnitListType<Time>);
static_assert(!UnitListType<Mass>);
static_assert(!UnitListType<Temperature>);
static_assert(!UnitListType<int>);
static_assert(UnitListType<UnitList<Distance>>);
static_assert(UnitListType<UnitList<Distance, Time>>);
static_assert(!UnitListType<Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(!UnitListType<std::ratio<1>>);
static_assert(!UnitListType<std::milli>);
static_assert(!UnitListType<std::mega>);
static_assert(!UnitListType<meters>);
static_assert(!UnitListType<::micromouse::millimeters>);

static_assert(!UnitSpec<Distance>);
static_assert(!UnitSpec<Time>);
static_assert(!UnitSpec<Mass>);
static_assert(!UnitSpec<Temperature>);
static_assert(!UnitSpec<int>);
static_assert(!UnitSpec<UnitList<Distance>>);
static_assert(!UnitSpec<UnitList<Distance, Time>>);
static_assert(UnitSpec<Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(!UnitSpec<std::ratio<1>>);
static_assert(!UnitSpec<std::milli>);
static_assert(!UnitSpec<std::mega>);
static_assert(!UnitSpec<meters>);
static_assert(!UnitSpec<::micromouse::millimeters>);

static_assert(!RatioSpec<Distance>);
static_assert(!RatioSpec<Time>);
static_assert(!RatioSpec<Mass>);
static_assert(!RatioSpec<Temperature>);
static_assert(!RatioSpec<int>);
static_assert(!RatioSpec<UnitList<Distance>>);
static_assert(!RatioSpec<UnitList<Distance, Time>>);
static_assert(!RatioSpec<Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(RatioSpec<std::ratio<1>>);
static_assert(RatioSpec<std::milli>);
static_assert(RatioSpec<std::mega>);
static_assert(!RatioSpec<meters>);
static_assert(!RatioSpec<::micromouse::millimeters>);

static_assert(!PhysicalSizeType<Distance>);
static_assert(!PhysicalSizeType<Time>);
static_assert(!PhysicalSizeType<Mass>);
static_assert(!PhysicalSizeType<Temperature>);
static_assert(!PhysicalSizeType<int>);
static_assert(!PhysicalSizeType<UnitList<Distance>>);
static_assert(!PhysicalSizeType<UnitList<Distance, Time>>);
static_assert(!PhysicalSizeType<Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(!PhysicalSizeType<std::ratio<1>>);
static_assert(!PhysicalSizeType<std::milli>);
static_assert(!PhysicalSizeType<std::mega>);
static_assert(PhysicalSizeType<meters>);
static_assert(PhysicalSizeType<::micromouse::millimeters>);

// UnitList - contains
static_assert(unit_list_contains_v<UnitList<Distance>, Distance>);
static_assert(!unit_list_contains_v<UnitList<>, Distance>);
static_assert(!unit_list_contains_v<UnitList<Time>, Distance>);
static_assert(unit_list_contains_v<UnitList<Time, Distance>, Distance>);
static_assert(unit_list_contains_v<UnitList<Distance, Time>, Distance>);

static_assert(unit_list_contains_any_v<UnitList<Distance>, Distance>);
static_assert(!unit_list_contains_any_v<UnitList<>, Distance>);
static_assert(!unit_list_contains_any_v<UnitList<Time>, Distance>);
static_assert(unit_list_contains_any_v<UnitList<Time, Distance>, Distance>);
static_assert(unit_list_contains_any_v<UnitList<Distance, Time>, Distance>);

static_assert(unit_list_contains_any_v<UnitList<Distance>, UnitList<Distance>>);
static_assert(unit_list_contains_any_v<UnitList<Distance>, UnitList<Distance, Time>>);
static_assert(unit_list_contains_any_v<UnitList<Distance>, UnitList<Time, Distance>>);
static_assert(!unit_list_contains_any_v<UnitList<>, UnitList<Distance>>);
static_assert(!unit_list_contains_any_v<UnitList<Time>, UnitList<Distance>>);
static_assert(unit_list_contains_any_v<UnitList<Time, Distance>, UnitList<Distance, Time>>);
static_assert(unit_list_contains_any_v<UnitList<Distance, Time>, UnitList<Distance>>);

static_assert(unit_list_contains_all_v<UnitList<Distance>, Distance>);
static_assert(!unit_list_contains_all_v<UnitList<>, Distance>);
static_assert(!unit_list_contains_all_v<UnitList<Time>, Distance>);
static_assert(unit_list_contains_all_v<UnitList<Time, Distance>, Distance>);
static_assert(unit_list_contains_all_v<UnitList<Distance, Time>, Distance>);

static_assert(unit_list_contains_all_v<UnitList<Distance>, UnitList<Distance>>);
static_assert(!unit_list_contains_all_v<UnitList<Distance>, UnitList<Distance, Time>>);
static_assert(!unit_list_contains_all_v<UnitList<Distance>, UnitList<Time, Distance>>);
static_assert(!unit_list_contains_all_v<UnitList<>, UnitList<Distance>>);
static_assert(!unit_list_contains_all_v<UnitList<Time>, UnitList<Distance>>);
static_assert(unit_list_contains_all_v<UnitList<Time, Distance>, UnitList<Distance, Time>>);
static_assert(unit_list_contains_all_v<UnitList<Distance, Time>, UnitList<Distance>>);

// UnitList - add
static_assert(std::is_same_v<unit_list_add<UnitList<Distance>, Time>, UnitList<Distance, Time>>);
static_assert(std::is_same_v<unit_list_add<UnitList<Distance>, Distance>, UnitList<Distance, Distance>>);
static_assert(std::is_same_v<unit_list_add<UnitList<Distance, Time>, Distance>, UnitList<Distance, Time, Distance>>);
static_assert(std::is_same_v<unit_list_add<UnitList<>, Distance>, UnitList<Distance>>);
static_assert(std::is_same_v<unit_list_add<UnitList<>, Distance, Time>, UnitList<Distance, Time>>);
static_assert(std::is_same_v<unit_list_add<UnitList<>, Distance, UnitList<Time>>, UnitList<Distance, Time>>);
static_assert(std::is_same_v<
              unit_list_add<UnitList<>, UnitList<Distance, Distance>, UnitList<Time>>,
              UnitList<Distance, Distance, Time>>);
static_assert(std::is_same_v<
              unit_list_add<UnitList<Distance>, Time, UnitList<Time, Distance>, Distance, UnitList<Distance, Time>>,
              UnitList<Distance, Time, Time, Distance, Distance, Distance, Time>>);

// UnitList - remove
static_assert(std::is_same_v<unit_list_remove<UnitList<Distance, Time>, Distance>, UnitList<Time>>);
static_assert(std::is_same_v<unit_list_remove_remainder<UnitList<Distance, Time>, Distance>, UnitList<>>);
static_assert(std::is_same_v<unit_list_remove<UnitList<Distance, Time>, Time>, UnitList<Distance>>);
static_assert(std::is_same_v<unit_list_remove_remainder<UnitList<Distance, Time>, Time>, UnitList<>>);
static_assert(std::is_same_v<unit_list_remove<UnitList<Distance, Distance>, Time>, UnitList<Distance, Distance>>);
static_assert(std::is_same_v<unit_list_remove_remainder<UnitList<Distance, Distance>, Time>, UnitList<Time>>);
static_assert(std::is_same_v<unit_list_remove<UnitList<Distance, Distance>, Distance>, UnitList<Distance>>);
static_assert(std::is_same_v<unit_list_remove_remainder<UnitList<Distance, Distance>, Distance>, UnitList<>>);

// UnitList - count (internal)
static_assert(detail::unit_list_count_v<UnitList<>, Distance> == 0);
static_assert(detail::unit_list_count_v<UnitList<Time>, Distance> == 0);
static_assert(detail::unit_list_count_v<UnitList<Distance>, Distance> == 1);
static_assert(detail::unit_list_count_v<UnitList<Distance, Time>, Distance> == 1);
static_assert(detail::unit_list_count_v<UnitList<Time, Distance>, Distance> == 1);
static_assert(detail::unit_list_count_v<UnitList<Distance, Distance>, Distance> == 2);
static_assert(detail::unit_list_count_v<UnitList<Distance, Time, Distance>, Distance> == 2);

// Unit - make_units
static_assert(std::is_same_v<make_units<>, Unit<UnitList<>>>);
static_assert(std::is_same_v<make_units<Distance>, Unit<UnitList<Distance>>>);
static_assert(std::is_same_v<make_units<Distance, Time>, Unit<UnitList<Distance, Time>>>);
static_assert(std::is_same_v<make_units<Distance, Distance>, Unit<UnitList<Distance, Distance>>>);

// Unit - equality
static_assert(SameUnitAs<make_units<Distance>, make_units<Distance>>);
static_assert(!SameUnitAs<make_units<Distance>, make_units<Time>>);
static_assert(SameUnitAs<make_units<Distance, Time>, make_units<Distance, Time>>);
static_assert(SameUnitAs<make_units<Distance, Time>, make_units<Time, Distance>>);
static_assert(SameUnitAs<make_units<>, make_units<>>);
static_assert(SameUnitAs<Unit<UnitList<Distance>, UnitList<Time>>, Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(SameUnitAs<
              Unit<UnitList<Mass, Distance>, UnitList<Time, Temperature>>,
              Unit<UnitList<Mass, Distance>, UnitList<Temperature, Time>>>);
static_assert(SameUnitAs<
              Unit<UnitList<Mass, Distance>, UnitList<Time, Temperature>>,
              Unit<UnitList<Distance, Mass>, UnitList<Time, Temperature>>>);
static_assert(SameUnitAs<
              Unit<UnitList<Mass, Distance>, UnitList<Time, Temperature>>,
              Unit<UnitList<Distance, Mass>, UnitList<Temperature, Time>>>);

// Unit - mul
static_assert(SameUnitAs<unit_mul<make_units<Distance>, make_units<Time>>, make_units<Distance, Time>>);
static_assert(SameUnitAs<
              unit_mul<make_units<Distance>, Unit<UnitList<Distance>, UnitList<Time>>>,
              Unit<UnitList<Distance, Distance>, UnitList<Time>>>);
static_assert(SameUnitAs<unit_mul<make_units<Distance>, Unit<UnitList<Time>, UnitList<Distance>>>, make_units<Time>>);

// Unit - div
static_assert(std::is_same_v<unit_div<Distance, Time>, Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(std::is_same_v<unit_div<make_units<Distance>, Time>, Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(std::is_same_v<unit_div<Distance, make_units<Time>>, Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(std::is_same_v<
              unit_div<make_units<Distance>, make_units<Time>>,
              Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(std::is_same_v<unit_div<make_units<>, Time>, Unit<UnitList<>, UnitList<Time>>>);
static_assert(std::is_same_v<unit_div<make_units<Distance, Time>, Time>, make_units<Distance>>);
static_assert(std::is_same_v<
              unit_div<make_units<Distance, Time, Time>, Unit<UnitList<>, UnitList<Time>>>,
              make_units<Distance, Time, Time, Time>>);
static_assert(std::is_same_v<
              unit_div<Unit<UnitList<Distance>, UnitList<Time, Time>>, Unit<UnitList<>, UnitList<Time>>>,
              Unit<UnitList<Distance>, UnitList<Time>>>);
static_assert(std::is_same_v<
              unit_div<Unit<UnitList<Distance>, UnitList<Time>>, Unit<UnitList<Distance>>>,
              Unit<UnitList<>, UnitList<Time>>>);

// unit_set (internal)
static_assert(std::is_same_v<detail::unit_set<UnitList<Distance>>, UnitList<Distance>>);
static_assert(std::is_same_v<detail::unit_set<UnitList<>>, UnitList<>>);
static_assert(std::is_same_v<detail::unit_set<UnitList<Distance, Time>>, UnitList<Distance, Time>>);
static_assert(std::is_same_v<detail::unit_set<UnitList<Time, Distance>>, UnitList<Time, Distance>>);
static_assert(std::is_same_v<detail::unit_set<UnitList<Distance, Distance>>, UnitList<Distance>>);
static_assert(units_equal_v<
              Unit<detail::unit_set<UnitList<Distance, Distance, Time, Distance, Time, Distance>>>,
              Unit<UnitList<Distance, Time>>>);
static_assert(std::is_same_v<
              detail::unit_set<UnitList<Distance, Distance, Time, Distance, Time, Distance, Time>>,
              UnitList<Distance, Time>>);

// to_string (internal)
static_assert(std::string_view(detail::to_string(123'456'789UZ)) == "123456789");
static_assert(std::string_view(detail::to_string(123'456)) == "123456");
static_assert(std::string_view(detail::to_string(-123'456)) == "-123456");

// get_symbol<PhysicalUnit>()
static_assert(get_symbol<Distance>() == "m"sv);
static_assert(get_symbol<Time>() == "s"sv);
template <PhysicalUnitType P>
struct test_physical_unit_symbol : std::bool_constant<get_symbol<P>()[0] == P::unit && get_symbol<P>()[1] == '\0'>
{};
static_assert(test_physical_unit_symbol<Distance>::value);
static_assert(test_physical_unit_symbol<Time>::value);
static_assert(test_physical_unit_symbol<Mass>::value);
static_assert(test_physical_unit_symbol<Temperature>::value);

// get_symbol<UnitList>()
static_assert(get_symbol<UnitList<Distance>>() == "m"sv);
static_assert(get_symbol<UnitList<Time>>() == "s"sv);
static_assert(get_symbol<UnitList<Distance, Time>>() == "ms"sv);
static_assert(get_symbol<UnitList<Distance, Distance, Distance>>() == "m^3"sv);
static_assert(get_symbol<UnitList<Distance, Distance, Time>>() == "m^2s"sv);
static_assert(get_symbol<UnitList<Distance, Time, Distance>>() == "m^2s"sv);
static_assert(get_symbol<UnitList<Time, Distance, Distance>>() == "m^2s"sv);

// get_symbol<Unit>()
static_assert(get_symbol<make_units<>>() == ""sv);
static_assert(get_symbol<make_units<Distance>>() == "m"sv);
static_assert(get_symbol<make_units<Time>>() == "s"sv);
static_assert(get_symbol<make_units<Distance, Time>>() == "ms"sv);
static_assert(get_symbol<make_units<Distance, Distance, Distance>>() == "m^3"sv);
static_assert(get_symbol<make_units<Distance, Distance, Time>>() == "m^2s"sv);
static_assert(get_symbol<make_units<Distance, Time, Distance>>() == "m^2s"sv);
static_assert(get_symbol<make_units<Time, Distance, Distance>>() == "m^2s"sv);
static_assert(get_symbol<Unit<UnitList<>, UnitList<Distance>>>() == "1/m"sv);
static_assert(get_symbol<Unit<UnitList<>, UnitList<Time>>>() == "Hz"sv);
static_assert(get_symbol<Unit<UnitList<>, UnitList<Distance, Time>>>() == "1/ms"sv);
static_assert(get_symbol<Unit<UnitList<>, UnitList<Distance, Distance, Distance>>>() == "1/m^3"sv);
static_assert(get_symbol<Unit<UnitList<>, UnitList<Distance, Distance, Time>>>() == "1/m^2s"sv);
static_assert(get_symbol<Unit<UnitList<>, UnitList<Distance, Time, Distance>>>() == "1/m^2s"sv);
static_assert(get_symbol<Unit<UnitList<>, UnitList<Time, Distance, Distance>>>() == "1/m^2s"sv);
static_assert(get_symbol<Unit<UnitList<Distance>, UnitList<Time>>>() == "m/s"sv);
static_assert(get_symbol<Unit<UnitList<Distance>, UnitList<Time, Time>>>() == "m/s^2"sv);
static_assert(get_symbol<Unit<UnitList<Distance, Distance>, UnitList<Time>>>() == "m^2/s"sv);
static_assert(get_symbol<Unit<UnitList<Distance, Distance>, UnitList<Time, Time>>>() == "m^2/s^2"sv);

// get_symbol<std::ratio>()
static_assert(get_symbol<std::ratio<1>>() == ""sv);
static_assert(get_symbol<std::atto>() == "a"sv);
static_assert(get_symbol<std::femto>() == "f"sv);
static_assert(get_symbol<std::pico>() == "p"sv);
static_assert(get_symbol<std::nano>() == "n"sv);
static_assert(get_symbol<std::micro>() == "u"sv);
static_assert(get_symbol<std::milli>() == "m"sv);
static_assert(get_symbol<std::centi>() == "c"sv);
static_assert(get_symbol<std::deci>() == "d"sv);
static_assert(get_symbol<std::deca>() == "da"sv);
static_assert(get_symbol<std::hecto>() == "h"sv);
static_assert(get_symbol<std::kilo>() == "k"sv);
static_assert(get_symbol<std::mega>() == "M"sv);
static_assert(get_symbol<std::giga>() == "G"sv);
static_assert(get_symbol<std::tera>() == "T"sv);
static_assert(get_symbol<std::peta>() == "P"sv);
static_assert(get_symbol<std::exa>() == "E"sv);
static_assert(get_symbol<std::ratio<60>>() == "60"sv);
static_assert(get_symbol<std::ratio<60, 3>>() == "20"sv);
static_assert(get_symbol<std::ratio<1, 60>>() == "1/60"sv);
static_assert(get_symbol<std::ratio<2, 60>>() == "1/30"sv);

// get_symbol<PhysicalSize>()
static_assert(get_symbol<PhysicalSize<int>>() == ""sv);
static_assert(get_symbol<meters>() == "m"sv);
static_assert(get_symbol<::micromouse::meters>() == "m"sv);
static_assert(get_symbol<centimeters>() == "cm"sv);
static_assert(get_symbol<::micromouse::centimeters>() == "cm"sv);
static_assert(get_symbol<millimeters>() == "mm"sv);
static_assert(get_symbol<::micromouse::millimeters>() == "mm"sv);
static_assert(get_symbol<meters_per_second>() == "m/s"sv);
static_assert(get_symbol<::micromouse::meters_per_second>() == "m/s"sv);
static_assert(get_symbol<PhysicalSize<int, make_units<Time>, std::ratio<60>>>() == "60s"sv);
static_assert(get_symbol<PhysicalSize<int, unit_div<make_units<>, Time>, std::mega>>() == "MHz"sv);
static_assert(get_symbol(1_m) == "m"sv);
static_assert(get_symbol(1_cm) == "cm"sv);
static_assert(get_symbol(5_mm / 1ms) == "m/s"sv);
static_assert(get_symbol(5_m / 1ms) == "km/s"sv);

TEST(PhysicalSizeTest, Casting)
{
    const auto x = 10_m;
    const auto y = 15_cm;

    ASSERT_EQ(x.count(), 10);
    ASSERT_EQ(unit_cast<std::milli>(x).count(), 10'000);
    ASSERT_EQ(unit_cast<millimeters>(x).count(), 10'000);
    ASSERT_FLOAT_EQ(unit_cast<float>(x).count(), 10.0f);
    ASSERT_FLOAT_EQ((unit_cast<float, std::milli>(x).count()), 1e4f);
    ASSERT_FLOAT_EQ(unit_cast<::micromouse::millimeters>(x).count(), 1e4f);

    ASSERT_EQ(unit_cast<millimeters>(x) + unit_cast<millimeters>(y), 10'150_mm);

    const auto t = PhysicalSize<std::int64_t, make_units<Time>, std::nano>(123);
    ASSERT_EQ(t.to_duration(), 123ns);
    ASSERT_EQ(t + 3ns, 126ns);
}

TEST(PhysicalSizeTest, UnitArithmetics)
{
    const auto x = 60_m;
    const auto y = 12ms;

    const auto speed = x / y;
    using speed_type = std::remove_const_t<decltype(speed)>;
    static_assert(std::is_same<speed_type::rep, std::int64_t>::value);
    static_assert(units_equal<speed_type::units, meters_per_second::units>::value);
    static_assert(std::ratio_equal<speed_type::ratio, std::kilo>::value);
    ASSERT_EQ(speed.count(), 5);

    const auto prod = x * y;
    using prod_type = std::remove_const_t<decltype(prod)>;
    static_assert(std::is_same<prod_type::rep, std::int64_t>::value);
    static_assert(units_equal<prod_type::units, make_units<Distance, Time>>::value);
    static_assert(std::ratio_equal<prod_type::ratio, std::milli>::value);
    ASSERT_EQ(prod.count(), 720);

    const auto pure = x / 6_m;
    using pure_type = std::remove_const_t<decltype(pure)>;
    static_assert(std::is_same<pure_type::rep, std::int64_t>::value);
    static_assert(units_equal<pure_type::units, make_units<>>::value);
    static_assert(std::ratio_equal<pure_type::ratio, std::ratio<1>>::value);
    ASSERT_EQ(pure.count(), 10);
    std::int64_t raw_pure = pure;
    ASSERT_EQ(raw_pure, 10);

    const auto kilo_pure = x / 5_mm;
    using kilo_pure_type = std::remove_const_t<decltype(kilo_pure)>;
    static_assert(std::is_same<kilo_pure_type::rep, std::int64_t>::value);
    static_assert(units_equal<kilo_pure_type::units, make_units<>>::value);
    static_assert(std::ratio_equal<kilo_pure_type::ratio, std::kilo>::value);
    ASSERT_EQ(kilo_pure.count(), 12);
    ASSERT_EQ(static_cast<std::int64_t>(kilo_pure), 12'000);
}

}  // namespace micromouse::tests

REGISTER_TEST_FILE(physical_size_tests);
