#ifndef MISC_UTILS_STRONGLY_TYPED_H
#define MISC_UTILS_STRONGLY_TYPED_H

#include <compare>
#include <concepts>
#include <type_traits>
#include <utility>

namespace micromouse
{

/**
 * @brief A base class for strongly typed wrappers.
 *
 * @example using MyStrongInt = StronglyTyped<int>
 *
 * @tparam T    Inner type.
 * @tparam Tag  Type separating tag (should always use the default).
 */
template <typename T, typename Tag = decltype([] {})>
    requires (std::is_trivially_copyable_v<T>)
class StronglyTyped
{
public:
    using type = T;

    constexpr StronglyTyped() noexcept = default;
    explicit constexpr StronglyTyped(type val) noexcept : value(val) {}
    template <typename... Args>
    explicit constexpr StronglyTyped(Args &&...args) noexcept : value(std::forward<Args>(args)...)
    {
    }

    constexpr StronglyTyped(const StronglyTyped &) noexcept = default;
    constexpr StronglyTyped &operator=(const StronglyTyped &) noexcept = default;
    constexpr StronglyTyped(StronglyTyped &&) noexcept = default;
    constexpr StronglyTyped &operator=(StronglyTyped &&) noexcept = default;
    ~StronglyTyped() noexcept = default;

    constexpr type get() const noexcept { return value; }
    explicit constexpr operator type() const noexcept { return get(); }

    template <std::constructible_from<type> U>
        requires (!std::is_same_v<type, U>)
    explicit constexpr operator U() const noexcept(noexcept(U(std::declval<type>())))
    {
        return static_cast<U>(get());
    }

    friend constexpr auto operator<=>(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value <=> rhs.value))
        requires (std::totally_ordered<type>)
    = default;

    friend constexpr auto operator<=>(const StronglyTyped &lhs, const type &rhs) noexcept(noexcept(lhs.value <=> rhs))
        requires (std::totally_ordered<type>)
    {
        return lhs.value <=> rhs;
    }

    friend constexpr StronglyTyped operator+(const StronglyTyped &val) noexcept(noexcept(+std::declval<type>()))
        requires (requires (T a) {
            { +a } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{+val.value};
    }
    friend constexpr StronglyTyped operator-(const StronglyTyped &val) noexcept(noexcept(-std::declval<type>()))
        requires (requires (T a) {
            { -a } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{-val.value};
    }
    friend constexpr StronglyTyped operator~(const StronglyTyped &val) noexcept(noexcept(~std::declval<type>()))
        requires (requires (T a) {
            { ~a } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{~val.value};
    }
    constexpr StronglyTyped &operator++() noexcept(noexcept(++std::declval<type &>()))
        requires (requires (T a) {
            { ++a };
        })
    {
        ++value;
        return *this;
    }
    constexpr StronglyTyped operator++(int) noexcept(noexcept(std::declval<type &>()++))
        requires (requires (T a) {
            { a++ } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{value++};
    }
    constexpr StronglyTyped &operator--() noexcept(noexcept(--std::declval<type &>()))
        requires (requires (T a) {
            { --a };
        })
    {
        --value;
        return *this;
    }
    constexpr StronglyTyped operator--(int) noexcept(noexcept(std::declval<type &>()--))
        requires (requires (T a) {
            { a-- } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{value--};
    }

    friend constexpr StronglyTyped operator+(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value + rhs.value))
        requires (requires (T a, T b) {
            { a + b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value + rhs.value};
    }
    friend constexpr StronglyTyped operator-(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value - rhs.value))
        requires (requires (T a, T b) {
            { a - b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value - rhs.value};
    }
    friend constexpr StronglyTyped operator*(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value * rhs.value))
        requires (requires (T a, T b) {
            { a * b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value * rhs.value};
    }
    friend constexpr StronglyTyped operator/(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value / rhs.value))
        requires (requires (T a, T b) {
            { a / b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value / rhs.value};
    }
    friend constexpr StronglyTyped operator%(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value % rhs.value))
        requires (requires (T a, T b) {
            { a % b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value % rhs.value};
    }
    friend constexpr StronglyTyped operator^(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value ^ rhs.value))
        requires (requires (T a, T b) {
            { a ^ b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value ^ rhs.value};
    }
    friend constexpr StronglyTyped operator&(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value & rhs.value))
        requires (requires (T a, T b) {
            { a & b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value & rhs.value};
    }
    friend constexpr StronglyTyped operator|(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value | rhs.value))
        requires (requires (T a, T b) {
            { a | b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value | rhs.value};
    }
    friend constexpr StronglyTyped operator<<(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value << rhs.value))
        requires (requires (T a, T b) {
            { a << b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value << rhs.value};
    }
    friend constexpr StronglyTyped operator>>(const StronglyTyped &lhs, const StronglyTyped &rhs)
        noexcept(noexcept(lhs.value >> rhs.value))
        requires (requires (T a, T b) {
            { a >> b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value >> rhs.value};
    }
    template <typename U>
    friend constexpr StronglyTyped operator<<(const StronglyTyped &lhs, const U &rhs)
        noexcept(noexcept(lhs.value << rhs))
        requires (requires (T a, U b) {
            { a << b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value << rhs};
    }
    template <typename U>
    friend constexpr StronglyTyped operator>>(const StronglyTyped &lhs, const U &rhs)
        noexcept(noexcept(lhs.value >> rhs))
        requires (requires (T a, U b) {
            { a >> b } -> std::convertible_to<type>;
        })
    {
        return StronglyTyped{lhs.value >> rhs};
    }

    constexpr StronglyTyped &operator+=(const StronglyTyped &other) noexcept(noexcept(value += other.value))
        requires (requires (T a, T b) {
            { a += b };
        })
    {
        value += other.value;
        return *this;
    }
    constexpr StronglyTyped &operator-=(const StronglyTyped &other) noexcept(noexcept(value -= other.value))
        requires (requires (T a, T b) {
            { a -= b };
        })
    {
        value -= other.value;
        return *this;
    }
    constexpr StronglyTyped &operator*=(const StronglyTyped &other) noexcept(noexcept(value *= other.value))
        requires (requires (T a, T b) {
            { a *= b };
        })
    {
        value *= other.value;
        return *this;
    }
    constexpr StronglyTyped &operator/=(const StronglyTyped &other) noexcept(noexcept(value /= other.value))
        requires (requires (T a, T b) {
            { a /= b };
        })
    {
        value /= other.value;
        return *this;
    }
    constexpr StronglyTyped &operator%=(const StronglyTyped &other) noexcept(noexcept(value %= other.value))
        requires (requires (T a, T b) {
            { a %= b };
        })
    {
        value %= other.value;
        return *this;
    }
    constexpr StronglyTyped &operator^=(const StronglyTyped &other) noexcept(noexcept(value ^= other.value))
        requires (requires (T a, T b) {
            { a ^= b };
        })
    {
        value ^= other.value;
        return *this;
    }
    constexpr StronglyTyped &operator&=(const StronglyTyped &other) noexcept(noexcept(value &= other.value))
        requires (requires (T a, T b) {
            { a &= b };
        })
    {
        value &= other.value;
        return *this;
    }
    constexpr StronglyTyped &operator|=(const StronglyTyped &other) noexcept(noexcept(value |= other.value))
        requires (requires (T a, T b) {
            { a |= b };
        })
    {
        value |= other.value;
        return *this;
    }
    constexpr StronglyTyped &operator<<=(const StronglyTyped &other) noexcept(noexcept(value <<= other.value))
        requires (requires (T a, T b) {
            { a <<= b };
        })
    {
        value <<= other.value;
        return *this;
    }
    constexpr StronglyTyped &operator>>=(const StronglyTyped &other) noexcept(noexcept(value >>= other.value))
        requires (requires (T a, T b) {
            { a >>= b };
        })
    {
        value >>= other.value;
        return *this;
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
    constexpr StronglyTyped operator!() const noexcept(noexcept(!std::declval<type>()))
        requires (std::is_same_v<type, bool> || requires (T a) {
            { !a };
            requires !std::is_same_v<decltype(!a), bool>;
            requires std::is_constructible_v<bool, decltype(!a)>;
        })
    {
        return StronglyTyped{!value};
    }

private:
    type value;
};

template <typename T>
struct is_strongly_typed : std::false_type
{};

template <typename T, typename Tag>
struct is_strongly_typed<StronglyTyped<T, Tag>> : std::true_type
{};

template <typename T>
inline constexpr auto is_strongly_typed_v = is_strongly_typed<T>::value;

template <typename T>
concept StrongType = is_strongly_typed_v<T>;

}  // namespace micromouse

#endif  // MISC_UTILS_STRONGLY_TYPED_H
