#ifndef MISC_UTILS_VALUE_RANGE_H
#define MISC_UTILS_VALUE_RANGE_H

#include "typing_utils.h"

#include <concepts>
#include <type_traits>

namespace micromouse
{

enum class Mode
{
    Closed,
    Inclusive = Closed,
    Open,
    Exclusive = Open,
    LeftOpen,
    LeftExclusive = LeftOpen,
    RightInclusive = LeftOpen,
    RightOpen,
    RightExclusive = RightOpen,
    LeftInclusive = RightOpen,
};

constexpr auto enum2str(Mode mode) noexcept
{
    switch (mode)
    {
    case Mode::Closed:
        return "Closed";
    case Mode::Open:
        return "Open";
    case Mode::LeftOpen:
        return "LeftOpen";
    case Mode::RightOpen:
        return "RightOpen";
    default:
        return "<invalid>";
    }
}

template <PartialArithmetic T, T Low, T High, Mode M, T Epsilon = T(1e-6)>
    requires (Low < High)
class ValueRange
{
public:
    using type = T;
    static constexpr Mode mode = M;
    static constexpr type low = Low;
    static constexpr type high = High;
    static constexpr type cycle = high - low;
    static constexpr type epsilon = Epsilon;
    static constexpr type clamp_epsilon = std::integral<T> ? 1 : epsilon;
    static constexpr bool is_open = mode == Mode::Open;
    static constexpr bool is_left_open = is_open || mode == Mode::LeftOpen;
    static constexpr bool is_right_open = is_open || mode == Mode::RightOpen;
    static constexpr bool is_closed = mode == Mode::Closed;

    // For now, declare all 4 for clarity (because conditionally declaring them is not trivial)
    // static constexpr std::enable_if_t<is_left_open, type> infimum = low;
    // static constexpr std::enable_if_t<!is_left_open, type> minimum = low;
    // static constexpr std::enable_if_t<is_right_open, type> supremum = high;
    // static constexpr std::enable_if_t<!is_right_open, type> maximum = high;
    static constexpr type infimum = low;
    static constexpr type minimum = low;
    static constexpr type supremum = high;
    static constexpr type maximum = high;

    static_assert(is_open == (is_left_open && is_right_open));
    static_assert(is_closed == (!is_left_open && !is_right_open));

    static constexpr type fix_cycle(type val) noexcept { return fix_high(fix_low(val)); }

    static constexpr type clamp(type val) noexcept { return clamp_high(clamp_low(val)); }

    static constexpr bool contains(type val) noexcept { return !out_of_range_low(val) && !out_of_range_high(val); }

private:
    static constexpr bool out_of_range_low(type val) noexcept
        requires (is_left_open)
    {
        return val <= infimum + epsilon;
    }
    static constexpr bool out_of_range_low(type val) noexcept
        requires (!is_left_open)
    {
        return val < minimum - epsilon;
    }
    static constexpr bool out_of_range_high(type val) noexcept
        requires (is_right_open)
    {
        return val >= supremum - epsilon;
    }
    static constexpr bool out_of_range_high(type val) noexcept
        requires (!is_right_open)
    {
        return val > maximum + epsilon;
    }

    static constexpr type fix_low(type val) noexcept
    {
        while (out_of_range_low(val))
        {
            val += cycle;
        }
        return val;
    }
    static constexpr type fix_high(type val) noexcept
    {
        while (out_of_range_high(val))
        {
            val -= cycle;
        }
        return val;
    }

    static constexpr type clamp_low(type val) noexcept
        requires (is_left_open)
    {
        return out_of_range_low(val) ? infimum + clamp_epsilon : val;
    }
    static constexpr type clamp_low(type val) noexcept
        requires (!is_left_open)
    {
        return out_of_range_low(val) ? minimum : val;
    }
    static constexpr type clamp_high(type val) noexcept
        requires (is_right_open)
    {
        return out_of_range_high(val) ? supremum - clamp_epsilon : val;
    }
    static constexpr type clamp_high(type val) noexcept
        requires (!is_right_open)
    {
        return out_of_range_high(val) ? maximum : val;
    }
};

struct unsafe_tag
{
} inline constexpr unsafe;

template <typename ValueRange, bool Cyclic = true>
class ConstrainedValue;

template <PartialArithmetic T, T Low, T High, Mode M, T Epsilon, bool Cyclic>
class ConstrainedValue<ValueRange<T, Low, High, M, Epsilon>, Cyclic>
{
public:
    using range_type = ValueRange<T, Low, High, M, Epsilon>;
    using type = typename range_type::type;
    static constexpr auto cyclic = Cyclic;

    constexpr ConstrainedValue() noexcept
        requires (range_type::contains(type{}))
    = default;
    constexpr ConstrainedValue() noexcept
        requires (!range_type::contains(type{}))
        : ConstrainedValue(type{})
    {
    }
    explicit constexpr ConstrainedValue(type val) noexcept : value(fix_value(val)) {}
    explicit constexpr ConstrainedValue(unsafe_tag, type val) noexcept : value(val) {}

    friend constexpr auto operator<=>(const ConstrainedValue &lhs, const ConstrainedValue &rhs) noexcept = default;
    friend constexpr auto operator<=>(const ConstrainedValue &lhs, type rhs) noexcept { return lhs.get() <=> rhs; }

    constexpr auto operator+() const noexcept { return ConstrainedValue(+value); }
    constexpr auto &operator++() noexcept
    {
        ++value;
        fix();
        return *this;
    }
    constexpr auto operator++(int) noexcept
    {
        auto old = ConstrainedValue(unsafe, value++);
        fix();
        return old;
    }
    constexpr auto operator-() const noexcept { return ConstrainedValue(-value); }
    constexpr auto &operator--() noexcept
    {
        --value;
        fix();
        return *this;
    }
    constexpr auto operator--(int) noexcept
    {
        auto old = ConstrainedValue(unsafe, value--);
        fix();
        return old;
    }
    friend constexpr auto operator+(const ConstrainedValue &lhs, const ConstrainedValue &rhs) noexcept
    {
        return ConstrainedValue(lhs.value + rhs.value);
    }
    friend constexpr auto operator-(const ConstrainedValue &lhs, const ConstrainedValue &rhs) noexcept
    {
        return ConstrainedValue(lhs.value - rhs.value);
    }
    friend constexpr auto operator*(const ConstrainedValue &lhs, const ConstrainedValue &rhs) noexcept
    {
        return ConstrainedValue(lhs.value * rhs.value);
    }
    friend constexpr auto operator/(const ConstrainedValue &lhs, const ConstrainedValue &rhs) noexcept
    {
        return ConstrainedValue(lhs.value / rhs.value);
    }

    constexpr auto &operator+=(const ConstrainedValue &other) noexcept
    {
        value += other.value;
        fix();
        return *this;
    }
    constexpr auto &operator-=(const ConstrainedValue &other) noexcept
    {
        value -= other.value;
        fix();
        return *this;
    }
    constexpr auto &operator*=(const ConstrainedValue &other) noexcept
    {
        value *= other.value;
        fix();
        return *this;
    }
    constexpr auto &operator/=(const ConstrainedValue &other) noexcept
    {
        value /= other.value;
        fix();
        return *this;
    }

    constexpr type get() const noexcept { return value; }
    constexpr operator type() const noexcept { return get(); }

    static constexpr type fix_value(type value) noexcept
        requires (cyclic)
    {
        return range_type::fix_cycle(value);
    }
    static constexpr type fix_value(type value) noexcept
        requires (!cyclic)
    {
        return range_type::clamp(value);
    }

private:
    constexpr void fix() noexcept { value = fix_value(value); }

    type value;
};

}  // namespace micromouse

#endif  // MISC_UTILS_VALUE_RANGE_H
