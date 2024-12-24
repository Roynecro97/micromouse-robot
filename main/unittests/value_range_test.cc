#include "misc_utils/value_range.h"

#include <misc_utils/angle.h>

#include "misc_utils_adapters.h"
#include "type_list.h"
#include "values.h"

#include <gtest/gtest.h>

#include <concepts>

#include <hack.h>

namespace micromouse::tests
{

template <PartialArithmetic T, T Low, T High>
struct ValueRangeParams;

template <typename>
struct ValueRangeTest;

template <PartialArithmetic T, T Low, T High>
class ValueRangeTest<ValueRangeParams<T, Low, High>> : public ::testing::Test
{
public:
    using type = T;
    static constexpr T low = Low;
    static constexpr T high = High;

    void SetUp() override
    {
        std::cout << "low = " << low << " | high = " << high
                  << " | floating point: " << (ExtendedFloatingPoint<T> ? "yes" : "no") << std::endl;
    }
};

using AngleRange2 = FloatingAngleRange<0.0f>;

using ValueRangeTypes = TypeList<
    ValueRangeParams<int, -10, 10>,
    ValueRangeParams<int, 5, 32>,
    ValueRangeParams<unsigned, 0, 100>,
    ValueRangeParams<unsigned, 5, 32>,
    ValueRangeParams<float, -10.0f, 10.0f>,
    ValueRangeParams<float, 10.0f, 75.43f>,
    ValueRangeParams<AngleRange::type, AngleRange::low, AngleRange::high>,
    ValueRangeParams<AngleRange2::type, AngleRange2::low, AngleRange2::high>>;

TYPED_TEST_SUITE(ValueRangeTest, ValueRangeTypes::types, );

template <PartialArithmetic T, T Low, T High, Mode M>
void value_range_test_impl()
{
    using range_type = ValueRange<T, Low, High, M>;
    using cv_cycle = ConstrainedValue<range_type, true>;
    using cv_clamp = ConstrainedValue<range_type, false>;

    static_assert(range_type::clamp_epsilon > T{});
    static_assert(!std::integral<T> || range_type::clamp_epsilon == 1);

    std::cout << "mode = " << M << std::endl;

    static_assert(range_type::contains(range_type::low) == !range_type::is_left_open);
    static_assert(range_type::contains(range_type::high) == !range_type::is_right_open);

    for (auto value : values_helper<T>::values)
    {
        EXPECT_GE(range_type::clamp(value), range_type::low);
        EXPECT_GE(range_type::fix_cycle(value), range_type::low);

        EXPECT_LE(range_type::clamp(value), range_type::high);
        EXPECT_LE(range_type::fix_cycle(value), range_type::high);

        EXPECT_EQ(cv_cycle(value).get(), range_type::fix_cycle(value));
        EXPECT_EQ(cv_clamp(value).get(), range_type::clamp(value));
    }
}

TYPED_TEST(ValueRangeTest, OpenRange)
{
    using type = TestFixture::type;
    using range_type = ValueRange<type, TestFixture::low, TestFixture::high, Mode::Open>;
    static_assert(std::is_same_v<typename range_type::type, type>);
    static_assert(range_type::low == TestFixture::low);
    static_assert(range_type::high == TestFixture::high);
    static_assert(range_type::mode == Mode::Open);
    static_assert(range_type::is_open);
    static_assert(range_type::is_left_open);
    static_assert(range_type::is_right_open);
    static_assert(!range_type::is_closed);

    value_range_test_impl<type, range_type::low, range_type::high, range_type::mode>();
}

TYPED_TEST(ValueRangeTest, ClosedRange)
{
    using type = TestFixture::type;
    using range_type = ValueRange<type, TestFixture::low, TestFixture::high, Mode::Closed>;
    static_assert(std::is_same_v<typename range_type::type, type>);
    static_assert(range_type::low == TestFixture::low);
    static_assert(range_type::high == TestFixture::high);
    static_assert(range_type::mode == Mode::Closed);
    static_assert(!range_type::is_open);
    static_assert(!range_type::is_left_open);
    static_assert(!range_type::is_right_open);
    static_assert(range_type::is_closed);

    value_range_test_impl<type, range_type::low, range_type::high, range_type::mode>();
}

TYPED_TEST(ValueRangeTest, LeftOpenRange)
{
    using type = TestFixture::type;
    using range_type = ValueRange<type, TestFixture::low, TestFixture::high, Mode::LeftOpen>;
    static_assert(std::is_same_v<typename range_type::type, type>);
    static_assert(range_type::low == TestFixture::low);
    static_assert(range_type::high == TestFixture::high);
    static_assert(range_type::mode == Mode::LeftOpen);
    static_assert(!range_type::is_open);
    static_assert(range_type::is_left_open);
    static_assert(!range_type::is_right_open);
    static_assert(!range_type::is_closed);

    value_range_test_impl<type, range_type::low, range_type::high, range_type::mode>();
}

TYPED_TEST(ValueRangeTest, RightOpenRange)
{
    using type = TestFixture::type;
    using range_type = ValueRange<type, TestFixture::low, TestFixture::high, Mode::RightOpen>;
    static_assert(std::is_same_v<typename range_type::type, type>);
    static_assert(range_type::low == TestFixture::low);
    static_assert(range_type::high == TestFixture::high);
    static_assert(range_type::mode == Mode::RightOpen);
    static_assert(!range_type::is_open);
    static_assert(!range_type::is_left_open);
    static_assert(range_type::is_right_open);
    static_assert(!range_type::is_closed);

    value_range_test_impl<type, range_type::low, range_type::high, range_type::mode>();
}

TEST(ValueRangeModeTest, ToString)
{
    static_assert(Mode::Closed == Mode::Inclusive);
    static_assert(Mode::Open == Mode::Exclusive);
    static_assert(Mode::LeftOpen == Mode::LeftExclusive);
    static_assert(Mode::LeftOpen == Mode::RightInclusive);
    static_assert(Mode::RightOpen == Mode::RightExclusive);
    static_assert(Mode::RightOpen == Mode::LeftInclusive);
    EXPECT_EQ(enum2str(Mode::Closed), "Closed");
    EXPECT_EQ(enum2str(Mode::Inclusive), "Closed");
    EXPECT_EQ(enum2str(Mode::Open), "Open");
    EXPECT_EQ(enum2str(Mode::Exclusive), "Open");
    EXPECT_EQ(enum2str(Mode::LeftOpen), "LeftOpen");
    EXPECT_EQ(enum2str(Mode::LeftExclusive), "LeftOpen");
    EXPECT_EQ(enum2str(Mode::RightInclusive), "LeftOpen");
    EXPECT_EQ(enum2str(Mode::RightOpen), "RightOpen");
    EXPECT_EQ(enum2str(Mode::RightExclusive), "RightOpen");
    EXPECT_EQ(enum2str(Mode::LeftInclusive), "RightOpen");
}

}  // namespace micromouse::tests

REGISTER_TEST_FILE(value_range_tests);
