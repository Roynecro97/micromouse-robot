#ifndef MISC_UTILS_BITS_PHYSICAL_UNIT_H
#define MISC_UTILS_BITS_PHYSICAL_UNIT_H

#include <type_traits>

namespace micromouse
{

template <char Shorthand>
struct PhysicalUnit
{
    static constexpr char unit = Shorthand;
};

template <typename>
struct is_physical_unit : std::false_type
{};

template <char Shorthand>
struct is_physical_unit<PhysicalUnit<Shorthand>> : std::true_type
{};

template <typename T>
inline constexpr auto is_physical_unit_v = is_physical_unit<T>::value;

template <typename T>
concept PhysicalUnitType = is_physical_unit_v<T>;

using Distance = PhysicalUnit<'m'>;
using Time = PhysicalUnit<'s'>;

}  // namespace micromouse

#endif  // MISC_UTILS_BITS_PHYSICAL_UNIT_H
