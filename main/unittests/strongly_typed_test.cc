#include "misc_utils/strongly_typed.h"

#include <misc_utils/angle.h>
#include <misc_utils/typing_utils.h>

#include "a.h"
#include "misc_utils_adapters.h"
#include "type_list.h"

#include <gtest/gtest.h>

#include <concepts>
#include <type_traits>

#include <hack.h>

namespace micromouse::tests
{

using SimpleStrongInt = StronglyTyped<int>;
using VeryStrongInt = StronglyTyped<SimpleStrongInt>;
using StrongBool = StronglyTyped<bool>;
using StrongA = StronglyTyped<A>;

struct custom_tag;
using TaggedStrongInt = StronglyTyped<int, custom_tag>;

struct CustomStrongInt : StronglyTypedBase<int, CustomStrongInt>
{
    using StronglyTypedBase<int, CustomStrongInt>::StronglyTypedBase;
};

static_assert(!std::is_same_v<StronglyTyped<int>, StronglyTyped<int>>, "Bad automatic tagging");
static_assert(!std::is_same_v<SimpleStrongInt, StronglyTyped<int>>, "Bad automatic tagging");
static_assert(std::is_same_v<SimpleStrongInt, SimpleStrongInt>, "Bad automatic tagging");

static_assert(std::is_same_v<StronglyTyped<int, custom_tag>, StronglyTyped<int, custom_tag>>, "Bad manual tagging");
static_assert(std::is_same_v<TaggedStrongInt, StronglyTyped<int, custom_tag>>, "Bad manual tagging");
static_assert(std::is_same_v<TaggedStrongInt, TaggedStrongInt>, "Bad manual tagging");

static_assert(!std::is_same_v<SimpleStrongInt, CustomStrongInt>, "Bad tagging");
static_assert(!std::is_same_v<SimpleStrongInt, TaggedStrongInt>, "Bad tagging");
static_assert(!std::is_same_v<CustomStrongInt, TaggedStrongInt>, "Bad tagging");

static_assert(is_strongly_typed_v<SimpleStrongInt>, "Trait missed a strong type");
static_assert(is_strongly_typed_v<TaggedStrongInt>, "Trait missed a strong type");
static_assert(is_strongly_typed_v<CustomStrongInt>, "Trait missed a strong type");
static_assert(is_strongly_typed_v<VeryStrongInt>, "Trait missed a strong type");
static_assert(is_strongly_typed_v<StrongBool>, "Trait missed a strong type");
static_assert(is_strongly_typed_v<StrongA>, "Trait missed a strong type");
static_assert(!is_strongly_typed_v<int>, "Trait detected a weak type as strong");
static_assert(!is_strongly_typed_v<float>, "Trait detected a weak type as strong");
static_assert(!is_strongly_typed_v<A>, "Trait detected a weak type as strong");

static_assert(StrongType<SimpleStrongInt>, "Concept missed a strong type");
static_assert(StrongType<TaggedStrongInt>, "Concept missed a strong type");
static_assert(StrongType<CustomStrongInt>, "Concept missed a strong type");
static_assert(StrongType<VeryStrongInt>, "Concept missed a strong type");
static_assert(StrongType<StrongBool>, "Concept missed a strong type");
static_assert(StrongType<StrongA>, "Concept missed a strong type");
static_assert(!StrongType<int>, "Concept detected a weak type as strong");
static_assert(!StrongType<float>, "Concept detected a weak type as strong");
static_assert(!StrongType<A>, "Concept detected a weak type as strong");

template <typename From, typename To>
struct is_only_explicitly_convertible_to
    : std::conjunction<std::negation<std::is_convertible<From, To>>, std::is_constructible<To, From>>
{};

template <typename T, typename U>
struct mutually_explicitly_convertible
    : std::conjunction<is_only_explicitly_convertible_to<T, U>, is_only_explicitly_convertible_to<U, T>>
{};

static_assert(mutually_explicitly_convertible<SimpleStrongInt, int>::value);
static_assert(mutually_explicitly_convertible<TaggedStrongInt, int>::value);
static_assert(mutually_explicitly_convertible<CustomStrongInt, int>::value);
static_assert(mutually_explicitly_convertible<VeryStrongInt, int>::value);
static_assert(mutually_explicitly_convertible<VeryStrongInt, SimpleStrongInt>::value);
static_assert(mutually_explicitly_convertible<StrongBool, bool>::value);
static_assert(mutually_explicitly_convertible<StrongA, A>::value);
static_assert(mutually_explicitly_convertible<StronglyTyped<float>, float>::value);
static_assert(mutually_explicitly_convertible<StronglyTyped<float>, int>::value);
static_assert(!mutually_explicitly_convertible<SimpleStrongInt, A>::value);
static_assert(!std::is_convertible<SimpleStrongInt, TaggedStrongInt>::value);
static_assert(!std::is_convertible<TaggedStrongInt, SimpleStrongInt>::value);
static_assert(!std::is_convertible<VeryStrongInt, TaggedStrongInt>::value);
static_assert(!std::is_convertible<TaggedStrongInt, VeryStrongInt>::value);

struct IntPoint
{
    int x;
    int y;

    constexpr IntPoint() noexcept = default;
    constexpr IntPoint(int x_coord, int y_coord) noexcept : x{x_coord}, y{y_coord} {}

    friend constexpr bool operator==(const IntPoint &, const IntPoint &) noexcept = default;
};
using StrongPoint = StronglyTyped<IntPoint>;

static_assert(mutually_explicitly_convertible<IntPoint, StrongPoint>::value);
static_assert(std::is_nothrow_constructible_v<StrongPoint, int, int>);

using BasicAngle = ConstrainedValue<AngleRange>;
using Angle = StronglyTyped<BasicAngle>;
struct CrtpAngle : StronglyTypedBase<BasicAngle, CrtpAngle>
{
    using StronglyTypedBase<BasicAngle, CrtpAngle>::StronglyTypedBase;
};

static_assert(!is_strongly_typed_v<BasicAngle>, "Trait detected a weak type as strong");
static_assert(is_strongly_typed_v<Angle>, "Trait missed a strong type");
static_assert(is_strongly_typed_v<CrtpAngle>, "Trait missed a strong type");

static_assert(!StrongType<BasicAngle>, "Concept detected a weak type as strong");
static_assert(StrongType<Angle>, "Concept missed a strong type");
static_assert(StrongType<CrtpAngle>, "Concept missed a strong type");

template <typename T>
constexpr auto default_value()
{
    return T{};
}

template <typename T>
constexpr auto some_value()
{
    return default_value<A>();
}
template <std::integral I>
auto some_value()
{
    return I(42);
}
template <std::floating_point F>
auto some_value()
{
    return F(123.45);
}
template <>
auto some_value<BasicAngle>()
{
    return BasicAngle{std::numbers::pi_v<BasicAngle::type> / 4};
}

template <StrongType T>
class StronglyTypedTestBase : public ::testing::Test
{
public:
    using strong_type = T;
    using type = T::type;
};

template <StrongType T>
using StronglyTypedTest = StronglyTypedTestBase<T>;

template <StrongType T>
using StronglyTypedArithmeticTest = StronglyTypedTestBase<T>;

using NonArithmeticTypes = TypeList<StrongA, StrongBool>;
using ArithmeticTypes = TypeList<
    SimpleStrongInt,
    StronglyTyped<float>,
    StronglyTyped<long>,
    StronglyTyped<short>,
    StronglyTyped<double>,
    Angle,
    CrtpAngle>;
using AllTypes = Join<ArithmeticTypes, NonArithmeticTypes>;

TYPED_TEST_SUITE(StronglyTypedTest, AllTypes::types, );
TYPED_TEST_SUITE(StronglyTypedArithmeticTest, ArithmeticTypes::types, );

TYPED_TEST(StronglyTypedTest, ConstructAndGet)
{
    using type = TestFixture::type;
    using strong_type = TestFixture::strong_type;
    static_assert(is_strongly_typed_v<strong_type>);

    type val = some_value<type>();
    strong_type strong(val);
    strong_type strong2(default_value<type>());
    ASSERT_EQ(strong.get(), val);
    ASSERT_EQ(strong, strong_type{val});
    ASSERT_EQ(*strong2, default_value<type>());
    ASSERT_EQ(strong2, strong_type{default_value<type>()});
    if (default_value<type>() == some_value<type>())
    {
        ASSERT_EQ(strong, strong2);
    }
    else
    {
        ASSERT_NE(strong, strong2);
    }
}

TYPED_TEST(StronglyTypedArithmeticTest, SimpleArithmetic)
{
    using type = TestFixture::type;
    using strong_type = TestFixture::strong_type;
    static_assert(is_strongly_typed_v<strong_type>);

    const auto a = some_value<type>();
    const auto b = static_cast<type>(a * 2);
    static_assert(std::is_same_v<decltype(a), const type> && std::is_same_v<decltype(a), const type> && std::is_same_v<decltype(b), const type>);

    const auto sa = strong_type(a);
    const auto sb = strong_type(b);
    static_assert(std::is_same_v<decltype(a), const type> && std::is_same_v<decltype(a), const type> && std::is_same_v<decltype(b), const type>);

    static_assert(std::is_same_v<decltype(sb + sa), strong_type>);
    EXPECT_EQ(b + a, (sb + sa).get());

    static_assert(std::is_same_v<decltype(sb - sa), strong_type>);
    EXPECT_EQ(b - a, (sb - sa).get());

    static_assert(std::is_same_v<decltype(sb * sa), strong_type>);
    EXPECT_EQ(b * a, (sb * sa).get());

    static_assert(std::is_same_v<decltype(sb / sa), strong_type>);
    EXPECT_EQ(b / a, (sb / sa).get());
}

}  // namespace micromouse::tests

REGISTER_TEST_FILE(strongly_typed_tests);
