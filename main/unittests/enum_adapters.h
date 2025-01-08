#ifndef UNITTESTS_ENUM_ADAPTERS_H
#define UNITTESTS_ENUM_ADAPTERS_H

#include <concepts>
#include <ostream>
#include <type_traits>

namespace micromouse
{

namespace detail
{
template <typename E>
concept StreamableEnum = std::is_enum_v<E> && requires (std::ostream &os, E val) {
    { os << enum2str(val) } -> std::same_as<std::ostream &>;
};
}  // namespace detail

std::ostream &operator<<(std::ostream &os, detail::StreamableEnum auto val)
{
    return os << enum2str(val);
}

}  // namespace micromouse

#endif  // UNITTESTS_ENUM_ADAPTERS_H
