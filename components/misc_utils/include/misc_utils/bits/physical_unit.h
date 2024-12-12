#ifndef MISC_UTILS_BITS_PHYSICAL_UNIT_H
#define MISC_UTILS_BITS_PHYSICAL_UNIT_H

#include "tstring.h"

#include <type_traits>

namespace micromouse
{

template <TString SI, TString Shorthand, typename Tag = decltype([] {})>
struct PhysicalUnit
{
    static constexpr const char *si_name = SI;
    static constexpr const char *si_unit = Shorthand;
};

template <typename>
struct is_physical_unit : std::false_type
{};

template <TString SI, TString Shorthand, typename Tag>
struct is_physical_unit<PhysicalUnit<SI, Shorthand, Tag>> : std::true_type
{};

template <typename T>
inline constexpr auto is_physical_unit_v = is_physical_unit<T>::value;

template <typename T>
concept PhysicalUnitType = is_physical_unit_v<T>;

using Distance = PhysicalUnit<"meter", "m">;
using Time = PhysicalUnit<"second", "s">;

}  // namespace micromouse

#endif  // MISC_UTILS_BITS_PHYSICAL_UNIT_H
