#ifndef UNITTESTS_MISC_UTILS_ADAPTERS_H
#define UNITTESTS_MISC_UTILS_ADAPTERS_H

#include <misc_utils/physical_size.h>
#include <misc_utils/strongly_typed.h>
#include <misc_utils/unit_symbols.h>
#include <misc_utils/value_range.h>

#include "enum_adapters.h"  // for the Mode enum

#include <concepts>
#include <ostream>
#include <type_traits>

namespace micromouse
{

namespace detail
{
template <typename T>
concept Streamable = requires (std::ostream &os, const T &val) {
    { os << val } -> std::same_as<std::ostream &>;
};

template <typename T>
concept StreamableStrongType = StrongType<T> && Streamable<typename T::type>;

template <typename T>
concept StreamablePhysicalSize = PhysicalSizeType<T> && Streamable<typename T::rep>;

}  // namespace detail

std::ostream &operator<<(std::ostream &os, const detail::StreamableStrongType auto &st)
{
    return os << "StronglyTyped{" << st.get() << "}";
}

template <typename R, bool C>
    requires (detail::Streamable<typename R::type>)
std::ostream &operator<<(std::ostream &os, const ConstrainedValue<R, C> &st)
{
    return os << "ConstrainedValue<low = " << R::low << ", high = " << R::high << ", mode = " << R::mode
              << ", cycle = " << std::boolalpha << C << ">{" << st.get() << "}";
}

std::ostream &operator<<(std::ostream &os, const detail::StreamablePhysicalSize auto &val)
{
    return os << val.count() << get_symbol<std::remove_cvref_t<decltype(val)>>();
}

}  // namespace micromouse

#endif  // UNITTESTS_MISC_UTILS_ADAPTERS_H
