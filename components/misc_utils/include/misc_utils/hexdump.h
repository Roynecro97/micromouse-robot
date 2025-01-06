#ifndef MISC_UTILS_HEXDUMP_H
#define MISC_UTILS_HEXDUMP_H

#include "bits/to_string.h"
#include "strongly_typed.h"

#include <array>
#include <bit>
#include <cctype>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <type_traits>

namespace micromouse::inline hd
{

/**
 * @brief Compact HexDumper configuration.
 */
struct HexDumpParams : StronglyTypedBase<std::uint32_t, HexDumpParams>
{
    using line_size_type = std::uint8_t;

    /**
     * @brief Default HexDumpParams values: show_space = true, real_address = false, bytes_per_line = 16,
     *        bytes_per_break = 8, non_print_sub = '.'
     *
     */
    constexpr HexDumpParams() noexcept : StronglyTypedBase{default_value} {}
    /**
     * @brief Initialize HexDumpParams directly from its inner representation.
     * @note This constructor is not meant to be used directly. Use the fluent methods below.
     *
     * @param val   The underlying memory representation.
     */
    explicit constexpr HexDumpParams(type val) noexcept : StronglyTypedBase{val} {}

    // Internal representation values
    static constexpr type show_space_bit{0x1};
    static constexpr type real_address_bit{0x2};
    static constexpr type non_print_sub_shift{8};
    static constexpr type bytes_per_break_shift{non_print_sub_shift + std::numeric_limits<unsigned char>::digits};
    static constexpr type bytes_per_line_shift{bytes_per_break_shift + std::numeric_limits<line_size_type>::digits};

    static_assert(std::popcount(show_space_bit) == 1, "bad bit flag");
    static_assert(std::popcount(real_address_bit) == 1, "bad bit flag");
    static_assert((show_space_bit & real_address_bit) == 0, "overlapping bit flags");
    static_assert(
        std::numeric_limits<type>::digits - std::countl_zero(show_space_bit | real_address_bit) <= non_print_sub_shift,
        "overlapping fields: bit flags & non_print_sub"
    );
    static_assert(
        non_print_sub_shift + std::numeric_limits<unsigned char>::digits <= bytes_per_break_shift,
        "overlapping fields: non_print_sub & bytes_per_break"
    );
    static_assert(
        bytes_per_break_shift + std::numeric_limits<line_size_type>::digits <= bytes_per_line_shift,
        "overlapping fields: bytes_per_break & bytes_per_line"
    );
    static_assert(
        bytes_per_line_shift + std::numeric_limits<line_size_type>::digits <= std::numeric_limits<type>::digits,
        "bit overflow"
    );
    static constexpr type default_value{
        (16 << bytes_per_line_shift) | (8 << bytes_per_break_shift) | ('.' << non_print_sub_shift) | show_space_bit
    };

    // Creation/modification methods
    /**
     * @brief Set or unset the show_space flag.
     *
     * @param val   New value for the show_space flag.
     * @return HexDumpParams& The current instance.
     */
    constexpr HexDumpParams &show_space(bool val) noexcept
    {
        if (val)
        {
            return operator|=(HexDumpParams{show_space_bit});
        }
        else
        {
            return operator&=(HexDumpParams{~show_space_bit});
        }
    }
    /**
     * @brief Get the value of the show_space flag.
     *
     * @return bool The value of the show_space flag.
     */
    constexpr bool show_space() const noexcept { return (get() & show_space_bit) != 0; }

    /**
     * @brief Set or unset the real_address flag.
     *
     * @param val   New value for the real_address flag.
     * @return HexDumpParams& The current instance.
     */
    constexpr HexDumpParams &real_address(bool val) noexcept
    {
        if (val)
        {
            return operator|=(HexDumpParams{real_address_bit});
        }
        else
        {
            return operator&=(HexDumpParams{~real_address_bit});
        }
    }
    /**
     * @brief Get the value of the real_address flag.
     *
     * @return bool The value of the real_address flag.
     */
    constexpr bool real_address() const noexcept { return (get() & real_address_bit) != 0; }

    /**
     * @brief Set the number of bytes per hexdump line.
     *
     * @param val   The number of bytes to render per line.
     * @return HexDumpParams& The current instance.
     */
    constexpr HexDumpParams &bytes_per_line(line_size_type val) noexcept
    {
        operator&=(HexDumpParams{~(type{std::numeric_limits<line_size_type>::max()} << bytes_per_line_shift)});
        return operator|=(HexDumpParams{static_cast<type>(val) << bytes_per_line_shift});
    }
    /**
     * @brief Get the number of bytes per hexdump line.
     *
     * @return line_size_type The number of bytes per hexdump line.
     */
    constexpr line_size_type bytes_per_line() const noexcept
    {
        return (get() >> bytes_per_line_shift) & std::numeric_limits<line_size_type>::max();
    }

    /**
     * @brief Set the number of bytes between wide breaks.
     * @note bytes_per_line should be a multiple of bytes_per_break.
     *
     * @param val   The number of bytes to render between wide breaks.
     * @return HexDumpParams& The current instance.
     */
    constexpr HexDumpParams &bytes_per_break(line_size_type val) noexcept
    {
        operator&=(HexDumpParams{~(type{std::numeric_limits<line_size_type>::max()} << bytes_per_break_shift)});
        return operator|=(HexDumpParams{static_cast<type>(val) << bytes_per_break_shift});
    }
    /**
     * @brief Get the number of bytes between wide breaks.
     *
     * @return line_size_type The number of bytes between wide breaks.
     */
    constexpr line_size_type bytes_per_break() const noexcept
    {
        return (get() >> bytes_per_break_shift) & std::numeric_limits<line_size_type>::max();
    }

    /**
     * @brief Set the character filler for non-printable bytes.
     *
     * @param c The character to use.
     * @return HexDumpParams& The current instance.
     */
    constexpr HexDumpParams &non_print_sub(char c) noexcept
    {
        operator&=(HexDumpParams{~(type{std::numeric_limits<unsigned char>::max()} << non_print_sub_shift)});
        return operator|=(HexDumpParams{static_cast<type>(c) << non_print_sub_shift});
    }
    /**
     * @brief Get the character filler for non-printable bytes.
     *
     * @return char The character representing non-printable bytes.
     */
    constexpr char non_print_sub() const noexcept
    {
        return static_cast<char>((get() >> non_print_sub_shift) & std::numeric_limits<unsigned char>::max());
    }

    /**
     * @brief The maximal length of a single output line using the current configuration.
     */
    constexpr std::size_t max_output_line() const noexcept
    {
        return
            /* max address width */ (std::numeric_limits<std::size_t>::digits / 4) + /* separator */ 2
            + (/* byte */ 2 + /* byte separator */ 1) * bytes_per_line()
            + (/* extra space ber break */ bytes_per_line() / bytes_per_break()) + /* text separator */ 1
            + /* ASCII view */ bytes_per_line() + /* line end */ 2;
    }
};

/**
 * @brief A type that can print a single hexdump line.
 */
template <typename F, HexDumpParams Params = HexDumpParams{}>
concept StringPrinter =
    std::is_invocable_v<F, const char *> || std::is_invocable_v<F, const char (&)[Params.max_output_line()]>;

/**
 * @brief A function object that creates hexdumps (in the style of the `hexdump -C` command).
 *
 * @tparam Params   Style configuration.
 * @tparam P        Print function.
 */
template <HexDumpParams Params, StringPrinter<Params> auto P>
    requires (
        Params.bytes_per_line() > 0 && Params.bytes_per_break() > 0
        && Params.bytes_per_line() % Params.bytes_per_break() == 0
    )
class HexDumper
{
public:
    static constexpr std::size_t bytes_per_line = Params.bytes_per_line();
    static constexpr std::size_t bytes_per_break = Params.bytes_per_break();
    static constexpr auto show_space = Params.show_space();
    static constexpr auto real_address = Params.real_address();
    static constexpr auto non_print_sub = Params.non_print_sub();
    static constexpr auto max_output_line = Params.max_output_line();

    /**
     * @brief Hexdump the provided memory.
     * @note Behavior is undefined if the memory range specified by buffer and size is invalid.
     *
     * @param buffer    Start address to dump from.
     * @param size      The amount of bytes to dump.
     */
    void operator()(const std::byte *buffer, std::size_t size) noexcept
    {
        const auto base_address = real_address ? reinterpret_cast<std::size_t>(buffer) : 0UZ;
        for (auto line = 0UZ; line < size; line += bytes_per_line)
        {
            auto pos = 0ZU;
            pos = write_at(pos, std::uint32_t(line + base_address));
            pos = write_at(pos, "  ");
            for (auto byte_idx = 0UZ; byte_idx < bytes_per_line; byte_idx++)
            {
                if (line + byte_idx < size)
                {
                    pos = write_at(pos, buffer[line + byte_idx]);
                    pos = write_at(pos, ' ');
                }
                else
                {
                    pos = write_at(pos, "   ");
                }
                if ((byte_idx + 1) % bytes_per_break == 0)
                {
                    pos = write_at(pos, ' ');
                }
            }
            pos = write_at(pos, '|');
            for (auto byte_idx = 0UZ; byte_idx < bytes_per_line; byte_idx++)
            {
                if (line + byte_idx < size)
                {
                    pos = write_at(pos, as_safe_print(buffer[line + byte_idx]));
                }
                else
                {
                    break;
                }
            }
            pos = write_at(pos, '|');
            write_at(pos, '\0');
            P(line_buffer);
        }
        write_at(write_at(0UZ, std::uint32_t(base_address + size)), '\0');
        P(line_buffer);
    }

    /**
     * @brief Hexdump the provided memory. (Overload for any pointer)
     * @note Behavior is undefined if the memory range specified by ptr and size is invalid.
     *
     * @param ptr   Start address to dump from.
     * @param size  The amount of bytes to dump.
     */
    void operator()(const void *ptr, std::size_t size) noexcept
    {
        operator()(static_cast<const std::byte *>(ptr), size);
    }

    /**
     * @brief Hexdump the provided object.
     *
     * @param obj   The object to dump.
     */
    void operator()(const auto &obj) noexcept
    {
        operator()(static_cast<const void *>(std::addressof(obj)), sizeof(obj));
    }

private:
    static char as_safe_print(std::uint8_t b)
    {
        if (std::isprint(b) && (!std::isspace(b) || (show_space && b == ' ')))
        {
            return static_cast<char>(b);
        }
        return non_print_sub;
    }

    static char as_safe_print(std::byte b) { return as_safe_print(std::to_integer<std::uint8_t>(b)); }

    std::size_t write_at(std::size_t pos, char value) noexcept
    {
        if (pos >= max_output_line - 1)
        {
            return pos;
        }
        line_buffer[pos] = value;
        return pos + 1;
    }

    template <std::size_t N>
    std::size_t write_at(std::size_t pos, const char (&value)[N]) noexcept
    {
        if (pos >= max_output_line - N)
        {
            return pos;
        }
        const auto size = (value[N - 1] == '\0') ? N - 1 : N;
        std::memcpy(line_buffer + pos, value, size);
        return pos + size;
    }

    std::size_t write_at(std::size_t pos, std::byte value) noexcept
    {
        write_at(pos, detail::digits[std::to_integer<std::size_t>((value & std::byte{0xF0}) >> 4)]);
        return write_at(pos + 1, detail::digits[std::to_integer<std::size_t>(value & std::byte{0x0F})]);
    }

    template <std::unsigned_integral U>
    std::size_t write_at(std::size_t pos, U value) noexcept
    {
        char hex[std::numeric_limits<U>::digits / 4];
        const auto nibble_end = std::rend(hex);
        for (auto nibble_it = std::rbegin(hex); nibble_it != nibble_end; ++nibble_it)
        {
            *nibble_it = detail::digits[value & 0xF];
            value >>= 4;
        }
        return write_at(pos, hex);
    }

    char line_buffer[max_output_line];
};

template <StringPrinter auto P = std::puts>
using HexDumperP = HexDumper<HexDumpParams{}, P>;

using HexDumperE = HexDumperP<[](const char *line) { std::fprintf(stderr, "%s\n", line); }>;

inline void hexdump(const std::byte *buffer, std::size_t size)
{
    HexDumperP<std::puts>{}(buffer, size);
}

inline void hexdump(const void *obj, std::size_t size)
{
    hexdump(static_cast<const std::byte *>(obj), size);
}

inline void hexdump(const auto &obj)
{
    hexdump(static_cast<const void *>(std::addressof(obj)), sizeof(obj));
}

}  // namespace micromouse::inline hd

#endif  // MISC_UTILS_HEXDUMP_H
