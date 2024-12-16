#ifndef MISC_UTILS_BITS_UNIT_LIST_H
#define MISC_UTILS_BITS_UNIT_LIST_H

#include "physical_unit.h"

#include <type_traits>

namespace micromouse
{

template <PhysicalUnitType...>
struct UnitList;

template <typename>
struct is_unit_list : std::false_type
{};

template <PhysicalUnitType... Ts>
struct is_unit_list<UnitList<Ts...>> : std::true_type
{};

template <typename T>
inline constexpr auto is_unit_list_v = is_unit_list<T>::value;

template <typename T>
concept UnitListType = is_unit_list_v<T>;

namespace detail
{
template <typename T>
concept UnitListOrPhysicalUnit = UnitListType<T> || PhysicalUnitType<T>;
}  // namespace detail

template <UnitListType, PhysicalUnitType>
struct unit_list_contains;

template <PhysicalUnitType... Ts, PhysicalUnitType U>
struct unit_list_contains<UnitList<Ts...>, U> : std::disjunction<std::is_same<Ts, U>...>
{};

template <UnitListType T, PhysicalUnitType U>
inline constexpr auto unit_list_contains_v = unit_list_contains<T, U>::value;

template <UnitListType, detail::UnitListOrPhysicalUnit>
struct unit_list_contains_any;

template <UnitListType T, PhysicalUnitType U>
struct unit_list_contains_any<T, U> : unit_list_contains<T, U>
{};

template <UnitListType T, PhysicalUnitType... Us>
struct unit_list_contains_any<T, UnitList<Us...>> : std::disjunction<unit_list_contains<T, Us>...>
{};

template <UnitListType T, detail::UnitListOrPhysicalUnit U>
inline constexpr auto unit_list_contains_any_v = unit_list_contains_any<T, U>::value;

template <UnitListType, detail::UnitListOrPhysicalUnit>
struct unit_list_contains_all;

template <UnitListType T, PhysicalUnitType U>
struct unit_list_contains_all<T, U> : unit_list_contains<T, U>
{};

template <UnitListType T, PhysicalUnitType... Us>
struct unit_list_contains_all<T, UnitList<Us...>> : std::conjunction<unit_list_contains<T, Us>...>
{};

template <UnitListType T, detail::UnitListOrPhysicalUnit U>
inline constexpr auto unit_list_contains_all_v = unit_list_contains_all<T, U>::value;

namespace detail
{

template <UnitListType, UnitListOrPhysicalUnit...>
struct unit_list_add_helper;

template <UnitListType T>
struct unit_list_add_helper<T>
{
    using type = T;
};

template <PhysicalUnitType... Ts, PhysicalUnitType U, UnitListOrPhysicalUnit... Rest>
struct unit_list_add_helper<UnitList<Ts...>, U, Rest...>
{
    using type = typename unit_list_add_helper<UnitList<Ts..., U>, Rest...>::type;
};

template <PhysicalUnitType... Ts, PhysicalUnitType... Us, UnitListOrPhysicalUnit... Rest>
struct unit_list_add_helper<UnitList<Ts...>, UnitList<Us...>, Rest...>
{
    using type = typename unit_list_add_helper<UnitList<Ts..., Us...>, Rest...>::type;
};

}  // namespace detail

template <UnitListType List, detail::UnitListOrPhysicalUnit... ToAdd>
using unit_list_add = typename detail::unit_list_add_helper<List, ToAdd...>::type;

namespace detail
{

template <UnitListType, UnitListOrPhysicalUnit>
struct unit_list_remove_helper;

template <PhysicalUnitType... Ts, PhysicalUnitType U>
struct unit_list_remove_helper<UnitList<U, Ts...>, U>
{
    using type = UnitList<Ts...>;
    using remainder = UnitList<>;
};

template <PhysicalUnitType T, PhysicalUnitType... Ts, PhysicalUnitType U>
    requires (unit_list_contains_v<UnitList<Ts...>, U>)
struct unit_list_remove_helper<UnitList<T, Ts...>, U>
{
    using type = unit_list_add<UnitList<T>, typename unit_list_remove_helper<UnitList<Ts...>, U>::type>;
    using remainder = UnitList<>;
};

template <PhysicalUnitType... Ts, PhysicalUnitType U>
    requires (!unit_list_contains_v<UnitList<Ts...>, U>)
struct unit_list_remove_helper<UnitList<Ts...>, U>
{
    using type = UnitList<Ts...>;
    using remainder = UnitList<U>;
};

template <UnitListType T>
struct unit_list_remove_helper<T, UnitList<>>
{
    using type = T;
    using remainder = UnitList<>;
};

template <UnitListType T, PhysicalUnitType U, PhysicalUnitType... Us>
struct unit_list_remove_helper<T, UnitList<U, Us...>>
{
    using step = unit_list_remove_helper<T, U>;
    using next_step = unit_list_remove_helper<typename step::type, UnitList<Us...>>;
    using type = typename next_step::type;
    using remainder = unit_list_add<typename step::remainder, typename next_step::remainder>;
};

}  // namespace detail

template <UnitListType List, detail::UnitListOrPhysicalUnit... ToRemove>
using unit_list_remove = typename detail::unit_list_remove_helper<List, ToRemove...>::type;

template <UnitListType List, detail::UnitListOrPhysicalUnit... ToRemove>
using unit_list_remove_remainder = typename detail::unit_list_remove_helper<List, ToRemove...>::remainder;

}  // namespace micromouse

#endif  // MISC_UTILS_BITS_UNIT_LIST_H
