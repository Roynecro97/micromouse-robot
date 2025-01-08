#ifndef MISC_UTILS_PHYSICAL_SIZE_H
#define MISC_UTILS_PHYSICAL_SIZE_H

#include "bits/physical_unit.h"
#include "bits/unit.h"
#include "bits/unit_list.h"
#include "typing_utils.h"
#include "value_range.h"

#include <chrono>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ratio>
#include <type_traits>

namespace micromouse
{

template <typename T>
struct is_ratio : std::false_type
{};

template <std::intmax_t Num, std::intmax_t Denom>
struct is_ratio<std::ratio<Num, Denom>> : std::true_type
{};

template <typename T>
inline constexpr auto is_ratio_v = is_ratio<T>::value;

template <typename T>
concept RatioSpec = is_ratio_v<T>;

template <ExtendedArithmetic Rep>
struct PhysicalSizeValues
{
    static constexpr Rep zero() noexcept { return Rep(0); }
    static constexpr Rep one() noexcept { return Rep(1); }
};

template <Arithmetic Rep>
struct PhysicalSizeValues<Rep>
{
    static constexpr Rep zero() noexcept { return Rep(0); }
    static constexpr Rep one() noexcept { return Rep(1); }
    static constexpr Rep min() noexcept { return std::numeric_limits<Rep>::lowest(); }
    static constexpr Rep max() noexcept { return std::numeric_limits<Rep>::max(); }
};

template <ArithmeticWrapper Rep>
struct PhysicalSizeValues<Rep> : PhysicalSizeValues<typename Rep::type>
{};

template <PartialArithmetic Rep, Rep Low, Rep High, Mode M, Rep Epsilon, bool Cyclic>
struct PhysicalSizeValues<ConstrainedValue<ValueRange<Rep, Low, High, M, Epsilon>, Cyclic>>
{
    static constexpr auto zero() noexcept
    {
        return ConstrainedValue<ValueRange<Rep, Low, High, M, Epsilon>, Cyclic>(0);
    }
    static constexpr auto one() noexcept { return ConstrainedValue<ValueRange<Rep, Low, High, M, Epsilon>, Cyclic>(1); }
    static constexpr auto min() noexcept
    {
        return ConstrainedValue<ValueRange<Rep, Low, High, M, Epsilon>, Cyclic>(
            ValueRange<Rep, Low, High, M, Epsilon>::clamp(Low)
        );
    }
    static constexpr auto max() noexcept
    {
        return ConstrainedValue<ValueRange<Rep, Low, High, M, Epsilon>, Cyclic>(
            ValueRange<Rep, Low, High, M, Epsilon>::clamp(High)
        );
    }
};

template <PartialArithmetic Rep, UnitSpec Units = make_units<>, RatioSpec Ratio = std::ratio<1>>
class PhysicalSize
{
public:
    using rep = Rep;
    using ratio = Ratio;
    using units = Units;

    constexpr PhysicalSize() noexcept = default;
    explicit(!units_equal_v<units, make_units<>>) constexpr PhysicalSize(rep val) noexcept : value(val) {}
    template <typename... Args>
        requires (std::is_constructible_v<rep, Args...>)
    explicit constexpr PhysicalSize(Args &&...args) noexcept(std::is_nothrow_constructible_v<rep, Args...>)
        : value(std::forward<Args>(args)...)
    {
    }
    template <std::convertible_to<rep> RepU, SameUnitAs<units> UnitsU>
        requires (!std::is_same_v<units, UnitsU>)
    constexpr PhysicalSize(const PhysicalSize<RepU, UnitsU, ratio> &other)
        noexcept(std::is_nothrow_convertible_v<RepU, rep>)
        : value(other.count())
    {
    }
    template <std::convertible_to<rep> RepU>
        requires (units_equal_v<units, make_units<Time>>)
    constexpr PhysicalSize(const std::chrono::duration<RepU, ratio> &dur)
        noexcept(std::is_nothrow_convertible_v<RepU, rep>)
        : value(dur.count())
    {
    }

    friend constexpr auto operator<=>(const PhysicalSize &lhs, const PhysicalSize &rhs) noexcept = default;
    template <std::totally_ordered_with<rep> RepU, SameUnitAs<units> UnitsU>
    friend constexpr auto operator<=>(const PhysicalSize &lhs, const PhysicalSize<RepU, UnitsU, ratio> &rhs) noexcept
    {
        return lhs.count() <=> rhs.count();
    }

    constexpr auto operator+() const noexcept { return PhysicalSize(+value); }
    constexpr auto &operator++() noexcept
    {
        ++value;
        return *this;
    }
    constexpr auto operator++(int) noexcept
    {
        auto old = PhysicalSize(value++);
        return old;
    }
    constexpr auto operator-() const noexcept { return PhysicalSize(-value); }
    constexpr auto &operator--() noexcept
    {
        --value;
        return *this;
    }
    constexpr auto operator--(int) noexcept
    {
        auto old = PhysicalSize(value--);
        return old;
    }
    friend constexpr auto operator+(const PhysicalSize &lhs, const PhysicalSize &rhs) noexcept
    {
        return PhysicalSize(lhs.count() + rhs.count());
    }
    friend constexpr auto operator-(const PhysicalSize &lhs, const PhysicalSize &rhs) noexcept
    {
        return PhysicalSize(lhs.count() - rhs.count());
    }

    constexpr auto &operator+=(const PhysicalSize &other) noexcept
    {
        value += other.value;
        return *this;
    }
    constexpr auto &operator-=(const PhysicalSize &other) noexcept
    {
        value -= other.value;
        return *this;
    }
    template <std::convertible_to<rep> T>
        requires requires (rep a, T b) {
            { a *= b };
        }
    constexpr auto &operator*=(const T &other) noexcept(noexcept(std::declval<rep &>() *= other))
    {
        value *= other;
        return *this;
    }
    template <std::convertible_to<rep> T>
        requires requires (rep a, T b) {
            { a /= b };
        }
    constexpr auto &operator/=(const T &other) noexcept(noexcept(std::declval<rep &>() /= other))
    {
        value /= other;
        return *this;
    }

    template <UnitSpec UnitsU, RatioSpec RatioU>
    friend constexpr auto operator*(const PhysicalSize &lhs, const PhysicalSize<rep, UnitsU, RatioU> &rhs) noexcept
    {
        return PhysicalSize<rep, unit_mul<units, UnitsU>, std::ratio_multiply<ratio, RatioU>>(
            lhs.count() * rhs.count()
        );
    }
    template <UnitSpec UnitsU, RatioSpec RatioU>
    friend constexpr auto operator/(const PhysicalSize &lhs, const PhysicalSize<rep, UnitsU, RatioU> &rhs) noexcept
    {
        return PhysicalSize<rep, unit_div<units, UnitsU>, std::ratio_divide<ratio, RatioU>>(lhs.count() / rhs.count());
    }

    template <PartialArithmetic RepU, RatioSpec RatioU>
        requires (requires (rep a, RepU b) {
            { a * b } -> std::convertible_to<rep>;
        })
    friend constexpr auto operator*(const PhysicalSize &lhs, const std::chrono::duration<RepU, RatioU> &rhs)
        noexcept(noexcept(std::declval<rep>() * std::declval<RepU>()))
    {
        return PhysicalSize<rep, unit_mul<units, Time>, std::ratio_multiply<ratio, RatioU>>(lhs.count() * rhs.count());
    }
    template <PartialArithmetic RepU, RatioSpec RatioU>
        requires (requires (rep a, RepU b) {
            { a / b } -> std::convertible_to<rep>;
        })
    friend constexpr auto operator/(const PhysicalSize &lhs, const std::chrono::duration<RepU, RatioU> &rhs)
        noexcept(noexcept(std::declval<rep>() / std::declval<RepU>()))
    {
        return PhysicalSize<rep, unit_div<units, Time>, std::ratio_divide<ratio, RatioU>>(lhs.count() / rhs.count());
    }

    template <PartialArithmetic RepU, RatioSpec RatioU>
        requires (requires (RepU a, rep b) {
            { a * b } -> std::convertible_to<rep>;
        })
    friend constexpr auto operator*(const std::chrono::duration<RepU, RatioU> &lhs, const PhysicalSize &rhs)
        noexcept(noexcept(std::declval<rep>() * std::declval<RepU>()))
    {
        return PhysicalSize<RepU, unit_mul<Time, units>, std::ratio_multiply<RatioU, ratio>>(lhs.count() * rhs.count());
    }
    template <PartialArithmetic RepU, RatioSpec RatioU>
        requires (requires (RepU a, rep b) {
            { a / b } -> std::convertible_to<rep>;
        })
    friend constexpr auto operator/(const std::chrono::duration<RepU, RatioU> &lhs, const PhysicalSize &rhs)
        noexcept(noexcept(std::declval<rep>() / std::declval<RepU>()))
    {
        return PhysicalSize<RepU, unit_div<Time, units>, std::ratio_divide<RatioU, ratio>>(lhs.count() / rhs.count());
    }

    template <std::convertible_to<rep> T>
        requires requires (rep a, T b) {
            { a * b } -> std::convertible_to<rep>;
        }
    friend constexpr auto operator*(const PhysicalSize &lhs, const T &rhs) noexcept(noexcept(lhs.count() * rhs))
    {
        return PhysicalSize(lhs.count() * rhs);
    }
    template <std::convertible_to<rep> T>
        requires requires (rep a, T b) {
            { a / b } -> std::convertible_to<rep>;
        }
    friend constexpr auto operator/(const PhysicalSize &lhs, const T &rhs) noexcept(noexcept(lhs.count() / rhs))
    {
        return PhysicalSize(lhs.count() / rhs);
    }

    template <std::convertible_to<rep> T>
        requires requires (T a, rep b) {
            { a * b } -> std::convertible_to<rep>;
        }
    friend constexpr auto operator*(const T &lhs, const PhysicalSize &rhs) noexcept(noexcept(lhs * rhs.count()))
    {
        return PhysicalSize(lhs * rhs.count());
    }
    template <std::convertible_to<rep> T>
        requires requires (T a, rep b) {
            { a / b } -> std::convertible_to<rep>;
        }
    friend constexpr auto operator/(const T &lhs, const PhysicalSize &rhs) noexcept(noexcept(lhs / rhs.count()))
    {
        return PhysicalSize<rep, unit_div<make_units<>, units>, std::ratio_divide<std::ratio<1>, ratio>>(
            lhs / rhs.count()
        );
    }

    constexpr rep count() const noexcept { return value; }

    constexpr std::chrono::duration<rep, ratio> to_duration() const noexcept
        requires (units_equal_v<units, make_units<Time>>)
    {
        return std::chrono::duration<rep, ratio>(count());
    }

    constexpr operator rep() const noexcept
        requires (units_equal_v<units, make_units<>> && std::ratio_equal_v<ratio, std::ratio<1>>)
    {
        return count();
    }

    explicit constexpr operator rep() const noexcept
        requires (units_equal_v<units, make_units<>> && !std::ratio_equal_v<ratio, std::ratio<1>>)
    {
        return count() * ratio::num / ratio::den;
    }

    static constexpr auto zero() noexcept { return PhysicalSize(PhysicalSizeValues<rep>::zero()); }
    static constexpr auto one() noexcept { return PhysicalSize(PhysicalSizeValues<rep>::one()); }
    static constexpr auto min() noexcept { return PhysicalSize(PhysicalSizeValues<rep>::min()); }
    static constexpr auto max() noexcept { return PhysicalSize(PhysicalSizeValues<rep>::max()); }

private:
    rep value{};
};

template <typename>
struct is_physical_size : std::false_type
{};

template <PartialArithmetic Rep, UnitSpec Units, RatioSpec Ratio>
struct is_physical_size<PhysicalSize<Rep, Units, Ratio>> : std::true_type
{};

template <typename T>
inline constexpr auto is_physical_size_v = is_physical_size<T>::value;

template <typename T>
concept PhysicalSizeType = is_physical_size_v<T>;

template <PhysicalSizeType ToSize, PartialArithmetic Rep, UnitSpec Units, RatioSpec Ratio>
    requires (units_equal_v<typename ToSize::units, Units>)
constexpr ToSize unit_cast(const PhysicalSize<Rep, Units, Ratio> &ps)
{
    using ToRep = ToSize::rep;
    using ToRatio = ToSize::ratio;
    using CF = std::ratio_divide<Ratio, ToRatio>;
    using CR = std::common_type_t<Rep, ToRep, std::intmax_t>;
    const auto cr_count = static_cast<CR>(ps.count());
    const auto cr_num = static_cast<CR>(CF::num);
    const auto cr_den = static_cast<CR>(CF::den);
    if constexpr (CF::num == 1)
    {
        if constexpr (CF::den == 1)
        {
            return ToSize(static_cast<ToRep>(ps.count()));
        }
        else
        {
            return ToSize(static_cast<ToRep>(cr_count / cr_den));
        }
    }
    else
    {
        if constexpr (CF::den == 1)
        {
            return ToSize(static_cast<ToRep>(cr_count * cr_num));
        }
        else
        {
            return ToSize(static_cast<ToRep>(cr_count * cr_num / cr_den));
        }
    }
}

template <PartialArithmetic Rep, RatioSpec Ratio>
PhysicalSize(const std::chrono::duration<Rep, Ratio> &) -> PhysicalSize<Rep, make_units<Time>, Ratio>;

template <PartialArithmetic Rep>
PhysicalSize(const Rep &) -> PhysicalSize<Rep>;

template <RatioSpec ToRatio, PartialArithmetic Rep, UnitSpec Units, RatioSpec Ratio>
constexpr auto unit_cast(const PhysicalSize<Rep, Units, Ratio> &ps)
{
    return unit_cast<PhysicalSize<Rep, Units, ToRatio>>(ps);
}

template <PartialArithmetic ToRep, PartialArithmetic Rep, UnitSpec Units, RatioSpec Ratio>
constexpr auto unit_cast(const PhysicalSize<Rep, Units, Ratio> &ps)
{
    return unit_cast<PhysicalSize<ToRep, Units, Ratio>>(ps);
}

template <PartialArithmetic ToRep, RatioSpec ToRatio, PartialArithmetic Rep, UnitSpec Units, RatioSpec Ratio>
constexpr auto unit_cast(const PhysicalSize<Rep, Units, Ratio> &ps)
{
    return unit_cast<PhysicalSize<ToRep, Units, ToRatio>>(ps);
}

using meters = PhysicalSize<float, make_units<Distance>>;
using centimeters = PhysicalSize<float, make_units<Distance>, std::centi>;
using millimeters = PhysicalSize<float, make_units<Distance>, std::milli>;
using meters_per_second = PhysicalSize<float, unit_div<Distance, Time>>;

inline namespace unit_literals
{

consteval auto operator""_m(long double val)
{
    return meters(val);
}

consteval auto operator""_m(unsigned long long val)
{
    return meters(val);
}

consteval auto operator""_cm(long double val)
{
    return centimeters(val);
}

consteval auto operator""_cm(unsigned long long val)
{
    return centimeters(val);
}

consteval auto operator""_mm(long double val)
{
    return millimeters(val);
}

consteval auto operator""_mm(unsigned long long val)
{
    return millimeters(val);
}

consteval auto operator""_mps(long double val)
{
    return meters_per_second(val);
}

consteval auto operator""_mps(unsigned long long val)
{
    return meters_per_second(val);
}

}  // namespace unit_literals

}  // namespace micromouse

#endif  // MISC_UTILS_PHYSICAL_SIZE_H
