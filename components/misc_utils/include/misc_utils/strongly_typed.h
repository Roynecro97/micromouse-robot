#ifndef MISC_UTILS_STRONGLY_TYPED_H
#define MISC_UTILS_STRONGLY_TYPED_H

#include <compare>
#include <concepts>
#include <type_traits>
#include <utility>

namespace micromouse
{

template <typename T>
struct is_strongly_typed;

/**
 * @brief A base class for strongly typed wrappers.
 *
 * @example struct MyStrongInt : StronglyTypedBase<int, MyStrongInt>
 *          {
 *              using StronglyTypedBase<int, MyStrongInt>::StronglyTypedBase;
 *          };
 *
 * @note Intended use is to publicly inherit from this struct and explicitly add a `using` directive for the
 *       constructor.
 *
 * @see StronglyTyped
 *
 * @tparam T    Inner type.
 * @tparam Self CRTP parameter.
 */
template <typename T, typename Self>
    requires (std::is_trivially_copyable_v<T>)
struct StronglyTypedBase
{
    using type = T;

    constexpr StronglyTypedBase() noexcept
        requires (std::is_trivially_default_constructible_v<T>)
    = default;
    constexpr StronglyTypedBase() noexcept
        requires (!std::is_trivially_default_constructible_v<T>)
        : StronglyTypedBase(type{})
    {
    }
    explicit constexpr StronglyTypedBase(type val) noexcept : value(val) {}
    template <typename... Args>
        requires (std::is_constructible_v<type, Args...>)
    explicit constexpr StronglyTypedBase(Args &&...args) noexcept(std::is_nothrow_constructible_v<type, Args...>)
        : value(std::forward<Args>(args)...)
    {
    }

    constexpr StronglyTypedBase(const StronglyTypedBase &) noexcept = default;
    constexpr StronglyTypedBase &operator=(const StronglyTypedBase &) noexcept = default;
    constexpr StronglyTypedBase(StronglyTypedBase &&) noexcept = default;
    constexpr StronglyTypedBase &operator=(StronglyTypedBase &&) noexcept = default;
    ~StronglyTypedBase() noexcept = default;

    constexpr type &get() & noexcept { return value; }
    constexpr type &&get() && noexcept { return std::move(value); }
    constexpr const type &get() const & noexcept { return value; }
    constexpr const type &&get() const && noexcept { return std::move(value); }
    constexpr type *operator->() noexcept { return &value; }
    constexpr const type *operator->() const noexcept { return &value; }
    constexpr type &operator*() noexcept { return value; }
    constexpr const type &operator*() const noexcept { return value; }
    explicit constexpr operator type() const noexcept { return get(); }

    template <std::constructible_from<type> U>
        requires (!std::is_same_v<type, U>)
    explicit constexpr operator U() const noexcept(noexcept(U(std::declval<type>())))
    {
        return static_cast<U>(get());
    }

    friend constexpr auto operator<=>(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value <=> rhs.value))
        requires (std::totally_ordered<type>)
    = default;

    friend constexpr auto operator<=>(const StronglyTypedBase &lhs, const type &rhs)
        noexcept(noexcept(lhs.value <=> rhs))
        requires (std::totally_ordered<type>)
    {
        return lhs.value <=> rhs;
    }

    friend constexpr Self operator+(const StronglyTypedBase &val) noexcept(noexcept(+std::declval<type>()))
        requires (requires (T a) {
            { +a } -> std::convertible_to<type>;
        })
    {
        return Self{+val.value};
    }
    friend constexpr Self operator-(const StronglyTypedBase &val) noexcept(noexcept(-std::declval<type>()))
        requires (requires (T a) {
            { -a } -> std::convertible_to<type>;
        })
    {
        return Self{-val.value};
    }
    friend constexpr Self operator~(const StronglyTypedBase &val) noexcept(noexcept(~std::declval<type>()))
        requires (requires (T a) {
            { ~a } -> std::convertible_to<type>;
        })
    {
        return Self{~val.value};
    }
    constexpr Self &operator++() noexcept(noexcept(++std::declval<type &>()))
        requires (requires (T a) {
            { ++a };
        })
    {
        ++value;
        return static_cast<Self &>(*this);
    }
    constexpr Self operator++(int) noexcept(noexcept(std::declval<type &>()++))
        requires (requires (T a) {
            { a++ } -> std::convertible_to<type>;
        })
    {
        return Self{value++};
    }
    constexpr Self &operator--() noexcept(noexcept(--std::declval<type &>()))
        requires (requires (T a) {
            { --a };
        })
    {
        --value;
        return static_cast<Self &>(*this);
    }
    constexpr Self operator--(int) noexcept(noexcept(std::declval<type &>()--))
        requires (requires (T a) {
            { a-- } -> std::convertible_to<type>;
        })
    {
        return Self{value--};
    }

    friend constexpr Self operator+(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value + rhs.value))
        requires (requires (T a, T b) {
            { a + b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value + rhs.value};
    }
    friend constexpr Self operator-(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value - rhs.value))
        requires (requires (T a, T b) {
            { a - b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value - rhs.value};
    }
    friend constexpr Self operator*(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value * rhs.value))
        requires (requires (T a, T b) {
            { a * b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value * rhs.value};
    }
    friend constexpr Self operator/(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value / rhs.value))
        requires (requires (T a, T b) {
            { a / b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value / rhs.value};
    }
    friend constexpr Self operator%(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value % rhs.value))
        requires (requires (T a, T b) {
            { a % b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value % rhs.value};
    }
    friend constexpr Self operator^(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value ^ rhs.value))
        requires (requires (T a, T b) {
            { a ^ b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value ^ rhs.value};
    }
    friend constexpr Self operator&(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value & rhs.value))
        requires (requires (T a, T b) {
            { a & b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value & rhs.value};
    }
    friend constexpr Self operator|(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value | rhs.value))
        requires (requires (T a, T b) {
            { a | b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value | rhs.value};
    }
    friend constexpr Self operator<<(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value << rhs.value))
        requires (requires (T a, T b) {
            { a << b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value << rhs.value};
    }
    friend constexpr Self operator>>(const StronglyTypedBase &lhs, const StronglyTypedBase &rhs)
        noexcept(noexcept(lhs.value >> rhs.value))
        requires (requires (T a, T b) {
            { a >> b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value >> rhs.value};
    }
    template <typename U>
    friend constexpr Self operator<<(const StronglyTypedBase &lhs, const U &rhs) noexcept(noexcept(lhs.value << rhs))
        requires (!is_strongly_typed<U>::value && requires (T a, U b) {
            { a << b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value << rhs};
    }
    template <typename U>
    friend constexpr Self operator>>(const StronglyTypedBase &lhs, const U &rhs) noexcept(noexcept(lhs.value >> rhs))
        requires (!is_strongly_typed<U>::value && requires (T a, U b) {
            { a >> b } -> std::convertible_to<type>;
        })
    {
        return Self{lhs.value >> rhs};
    }

    constexpr Self &operator+=(const StronglyTypedBase &other) noexcept(noexcept(value += other.value))
        requires (requires (T a, T b) {
            { a += b };
        })
    {
        value += other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator-=(const StronglyTypedBase &other) noexcept(noexcept(value -= other.value))
        requires (requires (T a, T b) {
            { a -= b };
        })
    {
        value -= other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator*=(const StronglyTypedBase &other) noexcept(noexcept(value *= other.value))
        requires (requires (T a, T b) {
            { a *= b };
        })
    {
        value *= other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator/=(const StronglyTypedBase &other) noexcept(noexcept(value /= other.value))
        requires (requires (T a, T b) {
            { a /= b };
        })
    {
        value /= other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator%=(const StronglyTypedBase &other) noexcept(noexcept(value %= other.value))
        requires (requires (T a, T b) {
            { a %= b };
        })
    {
        value %= other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator^=(const StronglyTypedBase &other) noexcept(noexcept(value ^= other.value))
        requires (requires (T a, T b) {
            { a ^= b };
        })
    {
        value ^= other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator&=(const StronglyTypedBase &other) noexcept(noexcept(value &= other.value))
        requires (requires (T a, T b) {
            { a &= b };
        })
    {
        value &= other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator|=(const StronglyTypedBase &other) noexcept(noexcept(value |= other.value))
        requires (requires (T a, T b) {
            { a |= b };
        })
    {
        value |= other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator<<=(const StronglyTypedBase &other) noexcept(noexcept(value <<= other.value))
        requires (requires (T a, T b) {
            { a <<= b };
        })
    {
        value <<= other.value;
        return static_cast<Self &>(*this);
    }
    constexpr Self &operator>>=(const StronglyTypedBase &other) noexcept(noexcept(value >>= other.value))
        requires (requires (T a, T b) {
            { a >>= b };
        })
    {
        value >>= other.value;
        return static_cast<Self &>(*this);
    }

    explicit constexpr operator bool() const noexcept(noexcept(static_cast<bool>(std::declval<type>())))
        requires (std::is_nothrow_constructible_v<bool, type> && !std::is_same_v<bool, type>)
    {
        return static_cast<bool>(value);
    }
    constexpr bool operator!() const noexcept(noexcept(!std::declval<type>()))
        requires (!std::is_same_v<type, bool> && requires (T a) {
            { !a } -> std::same_as<bool>;
        })
    {
        return !value;
    }
    constexpr Self operator!() const noexcept(noexcept(!std::declval<type>()))
        requires (std::is_same_v<type, bool> || requires (T a) {
            { !a };
            requires !std::is_same_v<decltype(!a), bool>;
            requires std::is_constructible_v<bool, decltype(!a)>;
        })
    {
        return Self{!value};
    }

    type value;
};

/**
 * @brief A base class for strongly typed wrappers.
 *
 * @example using MyStrongInt = StronglyTyped<int>
 *
 * @note Automatic tagging is problematic for type aliases defined in a header, for such types, consider using a manual
 *       tag or the StronglyTypedBase template.
 *
 * @see StronglyTypedBase
 *
 * @tparam T    Inner type.
 * @tparam Tag  Type separating tag.
 */
template <typename T, typename Tag = decltype([] {})>
    requires (std::is_trivially_copyable_v<T>)
struct StronglyTyped : StronglyTypedBase<T, StronglyTyped<T, Tag>>
{
    using StronglyTypedBase<T, StronglyTyped>::StronglyTypedBase;
};

template <typename T>
struct is_strongly_typed : std::false_type
{};

template <typename T>
    requires (std::is_base_of_v<StronglyTypedBase<typename T::type, T>, T>)
struct is_strongly_typed<T> : std::true_type
{};

template <typename T>
inline constexpr auto is_strongly_typed_v = is_strongly_typed<T>::value;

template <typename T>
concept StrongType = is_strongly_typed_v<T>;

}  // namespace micromouse

#endif  // MISC_UTILS_STRONGLY_TYPED_H
