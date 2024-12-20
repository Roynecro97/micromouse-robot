#ifndef UNITTESTS_A_H
#define UNITTESTS_A_H

#include <compare>
#include <ostream>

namespace micromouse::tests
{

struct A
{
    A operator+() const noexcept;
    A &operator++() noexcept;
    A operator++(int) const noexcept;
    A operator-() const noexcept;
    A &operator--() noexcept;
    A operator--(int) const noexcept;
    friend A operator+(const A &, const A &) noexcept;
    friend A operator-(const A &, const A &) noexcept;
    friend A operator*(const A &, const A &) noexcept;
    friend A operator/(const A &, const A &) noexcept;
    A &operator+=(const A &) noexcept;
    A &operator-=(const A &) noexcept;
    A &operator*=(const A &) noexcept;
    A &operator/=(const A &) noexcept;
    friend auto operator<=>(const A &, const A &) noexcept = default;
    friend bool operator==(const A &, const A &) noexcept = default;

    friend std::ostream &operator<<(std::ostream &os, const A &) { return os << 'A'; }
};

}  // namespace micromouse::tests

#endif  // UNITTESTS_A_H
