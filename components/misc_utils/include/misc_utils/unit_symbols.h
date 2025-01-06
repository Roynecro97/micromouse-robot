#ifndef MISC_UTILS_UNIT_SYMBOLS_H
#define MISC_UTILS_UNIT_SYMBOLS_H

#include "bits/to_string.h"
#include "bits/tstring.h"
#include "physical_size.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <limits>
#include <numeric>
#include <ratio>
#include <string_view>
#include <type_traits>

namespace micromouse
{

namespace detail
{

template <typename T>
concept HasSymbol = PhysicalUnitType<T> || UnitListType<T> || UnitSpec<T> || RatioSpec<T> || PhysicalSizeType<T>;

template <HasSymbol>
struct symbol_helper;

}  // namespace detail

template <detail::HasSymbol T>
consteval const char *get_symbol()
{
    return detail::symbol_helper<T>::symbol;
}

template <PhysicalSizeType T>
consteval auto get_symbol(const T &)
{
    return get_symbol<T>();
}

namespace detail
{

template <UnitListType>
struct reverse_unit_list_helper;

template <>
struct reverse_unit_list_helper<UnitList<>>
{
    using type = UnitList<>;
};

template <PhysicalUnitType T, PhysicalUnitType... Ts>
struct reverse_unit_list_helper<UnitList<T, Ts...>>
{
    using type = unit_list_add<typename reverse_unit_list_helper<UnitList<Ts...>>::type, T>;
};

template <UnitListType T>
using reverse_unit_list = typename reverse_unit_list_helper<T>::type;

template <UnitListType T>
struct make_reverse_unit_set
{};

template <>
struct make_reverse_unit_set<UnitList<>>
{
    using type = UnitList<>;
};

template <PhysicalUnitType T, PhysicalUnitType... Ts>
    requires (unit_list_contains_v<UnitList<Ts...>, T>)
struct make_reverse_unit_set<UnitList<T, Ts...>>
{
    using type = typename make_reverse_unit_set<UnitList<Ts...>>::type;
};

template <PhysicalUnitType T, PhysicalUnitType... Ts>
    requires (!unit_list_contains_v<UnitList<Ts...>, T>)
struct make_reverse_unit_set<UnitList<T, Ts...>>
{
    using type = unit_list_add<typename make_reverse_unit_set<UnitList<Ts...>>::type, T>;
};

template <UnitListType T>
struct make_unit_set
{
    using type = reverse_unit_list<typename make_reverse_unit_set<T>::type>;
};

template <UnitListType T>
using unit_set = make_unit_set<T>::type;

template <UnitListType, PhysicalUnitType, std::size_t C = 0>
struct unit_list_count_helper : std::integral_constant<std::size_t, C>
{};

template <PhysicalUnitType T, PhysicalUnitType... Ts, PhysicalUnitType U, std::size_t C>
struct unit_list_count_helper<UnitList<T, Ts...>, U, C>
    : unit_list_count_helper<UnitList<Ts...>, U, C + static_cast<unsigned>(std::is_same_v<T, U>)>
{};

template <UnitListType T, PhysicalUnitType U>
struct unit_list_count : unit_list_count_helper<T, U>
{};

template <UnitListType T, PhysicalUnitType U>
inline constexpr auto unit_list_count_v = unit_list_count<T, U>::value;

template <PhysicalUnitType T>
struct symbol_helper<T>
{
    static constexpr TString symbol{{T::unit, '\0'}};
};

template <PhysicalUnitType... Ts>
struct symbol_helper<UnitList<Ts...>>
{
    template <typename T>
    static constexpr auto mkpair() noexcept
    {
        return std::pair(get_symbol<T>(), unit_list_count_v<UnitList<Ts...>, T>);
    }

    template <UnitListType = unit_set<UnitList<Ts...>>>
    struct unique_helper;

    template <PhysicalUnitType... Us>
    struct unique_helper<UnitList<Us...>>
    {
        static constexpr auto mkextents_raw()
        {
            return std::array<std::pair<std::string_view, std::size_t>, sizeof...(Us)>{mkpair<Us>()...};
        }
    };

    static constexpr auto mkextents()
    {
        auto arr = unique_helper<>::mkextents_raw();
        std::sort(begin(arr), end(arr));
        return arr;
    }

    static constexpr auto extents = mkextents();

    static constexpr auto total_extent_len() noexcept
    {
        return std::accumulate(
            begin(extents),
            end(extents),
            0UZ,
            [](auto res, const auto &e) { return res + e.first.size() + ((e.second > 1) ? e.second + 1UZ : 0); }
        );
    }

    static consteval auto mksymbol()
    {
        TString<total_extent_len() + 1> str{};
        auto pos = std::begin(str.value);
        for (auto &&e : extents)
        {
            std::copy(begin(e.first), end(e.first), pos);
            pos += e.first.size();
            if (e.second > 1)
            {
                *pos = '^';
                auto pow = to_string(e.second);
                auto pow_view = std::string_view(pow);
                std::ranges::copy(begin(pow_view), end(pow_view), pos + 1);
                pos += pow_view.size() + 1;
            }
        }
        return str;
    }

    static constexpr auto symbol = mksymbol();
};

template <UnitSpec T>
struct symbol_helper<T>
{
    static constexpr auto num_sym = std::string_view(symbol_helper<typename T::num>::symbol);
    static constexpr auto den_sym = std::string_view(symbol_helper<typename T::den>::symbol);

    static consteval auto get_symbol_impl()
    {
        TString<std::max(num_sym.size(), 1UZ) + den_sym.size() + ((den_sym.size() > 0) ? 2 : 1)> res{};
        auto pos = std::begin(res.value);
        if (num_sym.size() > 0)
        {
            std::copy(begin(num_sym), end(num_sym), pos);
            pos += num_sym.size();
        }
        else
        {
            *pos = '1';
            ++pos;
        }
        if (den_sym.size() > 0)
        {
            *pos = '/';
            std::copy(begin(den_sym), end(den_sym), pos + 1);
        }
        return res;
    }

    static constexpr auto symbol = get_symbol_impl();
};

template <RatioSpec T>
struct symbol_helper<T>
{
    static constexpr auto num = detail::to_string(T::num);
    static constexpr auto den = detail::to_string(T::den);
    static constexpr auto num_view = std::string_view(num);
    static constexpr auto den_view = std::string_view(den);

    static consteval auto get_symbol_impl()
    {
        if constexpr (T::num == 0)
        {
            return TString("0");
        }
        else
        {
            TString<num_view.size() + ((T::den != 1) ? den_view.size() + 2 : 1)> res;
            auto pos = std::begin(res.value);
            std::copy(begin(num_view), end(num_view), pos);
            if (T::den != 1)
            {
                pos += num_view.size();
                *pos = '/';
                std::copy(begin(den_view), end(den_view), pos + 1);
            }
            return res;
        }
    }

    static constexpr auto symbol = get_symbol_impl();
};

template <PhysicalSizeType T>
struct symbol_helper<T>
{
    static constexpr auto ratio = std::string_view(get_symbol<typename T::ratio>());
    static constexpr auto units = std::string_view(get_symbol<typename T::units>());

    static consteval auto get_symbol_impl()
    {
        TString<ratio.size() + units.size() + 1> res{};
        std::copy(begin(ratio), end(ratio), res.value);
        std::copy(begin(units), end(units), res.value + ratio.size());
        return res;
    }

    static constexpr auto symbol = get_symbol_impl();
};

}  // namespace detail

// clang-format off
template <> consteval const char *get_symbol<make_units<>>() { return ""; }
template <> consteval const char *get_symbol<unit_div<make_units<>, Time>>() { return "Hz"; }

template <> consteval const char *get_symbol<std::atto>() { return "a"; }
template <> consteval const char *get_symbol<std::femto>() { return "f"; }
template <> consteval const char *get_symbol<std::pico>() { return "p"; }
template <> consteval const char *get_symbol<std::nano>() { return "n"; }
template <> consteval const char *get_symbol<std::micro>() { return "u"; }
template <> consteval const char *get_symbol<std::milli>() { return "m"; }
template <> consteval const char *get_symbol<std::centi>() { return "c"; }
template <> consteval const char *get_symbol<std::deci>() { return "d"; }
template <> consteval const char *get_symbol<std::deca>() { return "da"; }
template <> consteval const char *get_symbol<std::hecto>() { return "h"; }
template <> consteval const char *get_symbol<std::kilo>() { return "k"; }
template <> consteval const char *get_symbol<std::mega>() { return "M"; }
template <> consteval const char *get_symbol<std::giga>() { return "G"; }
template <> consteval const char *get_symbol<std::tera>() { return "T"; }
template <> consteval const char *get_symbol<std::peta>() { return "P"; }
template <> consteval const char *get_symbol<std::exa>() { return "E"; }
template <> consteval const char *get_symbol<std::ratio<1>>() { return ""; }
// clang-format on

}  // namespace micromouse

#endif  // MISC_UTILS_UNIT_SYMBOLS_H
