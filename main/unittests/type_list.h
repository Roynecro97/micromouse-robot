#ifndef UNITTEST_TYPE_LIST_H
#define UNITTEST_TYPE_LIST_H

#include <gtest/gtest.h>  // for ::testing::Types<...>

#include <type_traits>

namespace micromouse::tests
{

// TypeList - A list of types
template <typename... Ts>
struct TypeList
{
    using types = ::testing::Types<Ts...>;
};

template <typename T>
struct is_type_list : std::false_type
{};

template <typename... Ts>
struct is_type_list<TypeList<Ts...>> : std::true_type
{};

template <typename T>
inline constexpr auto is_type_list_v = is_type_list<T>::value;

template <typename T>
concept TypeListType = is_type_list_v<T>;

template <typename T>
concept TypeListManipulator = TypeListType<typename T::as_list>;

template <typename T>
concept ExtendedTypeList = TypeListType<T> || TypeListManipulator<T>;

static_assert(ExtendedTypeList<TypeList<int, float>>);
static_assert(TypeListType<TypeList<int, float>>);
static_assert(!TypeListManipulator<TypeList<int, float>>);

// Join - Concatenates `TypeList`s (or manipulators)
template <ExtendedTypeList, ExtendedTypeList>
struct Join;

template <TypeListManipulator Lhs, TypeListManipulator Rhs>
struct Join<Lhs, Rhs> : Join<typename Lhs::as_list, typename Rhs::as_list>
{};

template <TypeListType Lhs, TypeListManipulator Rhs>
struct Join<Lhs, Rhs> : Join<Lhs, typename Rhs::as_list>
{};

template <TypeListManipulator Lhs, TypeListType Rhs>
struct Join<Lhs, Rhs> : Join<typename Lhs::as_list, Rhs>
{};

template <typename... Ts, typename... Us>
struct Join<TypeList<Ts...>, TypeList<Us...>>
{
    using as_list = TypeList<Ts..., Us...>;
    using types = typename as_list::types;
};

static_assert(ExtendedTypeList<Join<TypeList<int, float>, TypeList<bool>>>);
static_assert(!TypeListType<Join<TypeList<int, float>, TypeList<bool>>>);
static_assert(TypeListManipulator<Join<TypeList<int, float>, TypeList<bool>>>);
static_assert(std::is_same_v<typename Join<TypeList<int, float>, TypeList<bool>>::as_list, TypeList<int, float, bool>>);

// Apply - Apply a manipulator on a type, works for both `std::add_const` and `std::add_const_t`
template <template <typename> class Operation, typename Type>
struct Apply
{
    template <typename T, typename = void>
    struct Helper
    {
        using type = T;
    };

    template <typename T>
    struct Helper<T, std::void_t<typename T::type>>
    {
        using type = typename T::type;
    };

    using type = typename Helper<Operation<Type>>::type;
};

template <template <typename> class Operation, typename T>
using ApplyType = typename Apply<Operation, T>::type;

// Transform - Apply a manipulator on a `TypeList`
template <ExtendedTypeList, template <typename> class>
struct Transform;

template <TypeListManipulator List, template <typename> class Operation>
struct Transform<List, Operation> : Transform<typename List::as_list, Operation>
{};

template <typename... Ts, template <typename> class Operation>
struct Transform<TypeList<Ts...>, Operation>
{
    using as_list = TypeList<ApplyType<Operation, Ts>...>;
    using types = typename as_list::types;
};

static_assert(ExtendedTypeList<Transform<TypeList<int, float>, std::add_const>>);
static_assert(!TypeListType<Transform<TypeList<int, float>, std::add_const>>);
static_assert(TypeListManipulator<Transform<TypeList<int, float>, std::add_const>>);
static_assert(std::is_same_v<
              typename Transform<TypeList<int, float>, std::add_const>::as_list,
              TypeList<const int, const float>>);

static_assert(ExtendedTypeList<Transform<TypeList<int, float>, std::add_const_t>>);
static_assert(!TypeListType<Transform<TypeList<int, float>, std::add_const_t>>);
static_assert(TypeListManipulator<Transform<TypeList<int, float>, std::add_const_t>>);
static_assert(std::is_same_v<
              typename Transform<TypeList<int, float>, std::add_const_t>::as_list,
              TypeList<const int, const float>>);
}  // namespace micromouse::tests

#endif  // UNITTEST_TYPE_LIST_H
