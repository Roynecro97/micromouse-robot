#ifndef MAIN_AVERAGE_FILTER_H
#define MAIN_AVERAGE_FILTER_H

#include <array>
#include <type_traits>

template <typename T>
concept unqualified_arithmetic = std::is_arithmetic_v<T> && !std::is_const_v<T> && !std::is_volatile_v<T>;

template <unqualified_arithmetic T, std::size_t N, unqualified_arithmetic AvgType = T>
class avg_filter
{
    static_assert(N > 0);

public:
    constexpr avg_filter() noexcept = default;
    explicit constexpr avg_filter(T value) noexcept : m_sum{static_cast<AvgType>(value) * N} { m_history.fill(value); }
    // TODO: allow more complex history
    constexpr avg_filter(const avg_filter &) noexcept = default;
    constexpr avg_filter(avg_filter &&) noexcept = default;
    constexpr avg_filter &operator=(const avg_filter &) noexcept = default;
    constexpr avg_filter &operator=(avg_filter &&) noexcept = default;
    ~avg_filter() noexcept = default;

    constexpr avg_filter &operator=(T value) noexcept
    {
        update(value);
        return *this;
    }
    constexpr operator AvgType() const noexcept { return avg(); }

    constexpr void update(T value) noexcept
    {
        m_sum -= m_history[m_i];
        m_sum += value;
        m_history[m_i] = value;
        m_i++;  // Or: m_i = (m_i + 1) % N;
        if (m_i == N)
        {
            m_i = 0;
        }
    }

    constexpr AvgType avg() const noexcept { return m_sum / N; }

private:
    std::array<T, N> m_history{};
    std::size_t m_i{0};
    AvgType m_sum{};
};

#endif  // MAIN_AVERAGE_FILTER_H
