#include "maze_solver/direction.h"

#include <misc_utils/angle.h>

#include "enum_adapters.h"

#include <gtest/gtest.h>

#include <numbers>
#include <type_traits>
#include <unordered_set>

#include <hack.h>

namespace micromouse::tests
{

TEST(RelativeDirectionTest, Invert)
{
    EXPECT_EQ(invert(RelativeDirection::Front), RelativeDirection::Back);
    EXPECT_EQ(invert(RelativeDirection::Back), RelativeDirection::Front);
    EXPECT_EQ(invert(RelativeDirection::Left), RelativeDirection::Right);
    EXPECT_EQ(invert(RelativeDirection::Right), RelativeDirection::Left);

    for (auto rd : relative_directions)
    {
        EXPECT_EQ(invert(invert(rd)), rd);
    }
}

TEST(RelativeDirectionTest, ToString)
{
    EXPECT_EQ(enum2str(RelativeDirection::Front), "Front");
    EXPECT_EQ(enum2str(RelativeDirection::Back), "Back");
    EXPECT_EQ(enum2str(RelativeDirection::Left), "Left");
    EXPECT_EQ(enum2str(RelativeDirection::Right), "Right");
}

template <unsigned Turns, Direction (*TurnFunc)(Direction)>
    requires (Turns > 0)
void test_turn_cycle_impl()
{
    std::unordered_set<Direction> seen;
    seen.reserve(std::extent_v<decltype(primary_directions)>);
    for (auto d : primary_directions)
    {
        seen.clear();
        auto curr = d;
        for (unsigned i = 0; i < Turns; ++i)
        {
            ASSERT_TRUE(seen.insert(curr).second) << "cycle ended too early (after " << i << " steps, expected "
                                                  << Turns << ", started at " << d << " and got to " << curr << ")";
            curr = TurnFunc(curr);
        }
        ASSERT_EQ(curr, d) << "cycle didn't end after " << Turns << " steps (started at " << d << " and got to " << curr
                           << ")";
        ASSERT_EQ(seen.size(), Turns) << "not enough unique steps";
    }
}

TEST(DirectionTest, TurnCycle)
{
    test_turn_cycle_impl<2, turn_back>();
    test_turn_cycle_impl<4, turn_left>();
    test_turn_cycle_impl<4, turn_right>();
}

TEST(DirectionTest, TurnRel)
{
    for (auto d : primary_directions)
    {
        EXPECT_EQ(turn(d, RelativeDirection::Front), d);
        EXPECT_EQ(turn(d, RelativeDirection::Back), turn_back(d));
        EXPECT_EQ(turn(d, RelativeDirection::Left), turn_left(d));
        EXPECT_EQ(turn(d, RelativeDirection::Right), turn_right(d));
    }
}

TEST(DirectionTest, ToDegrees)
{
    EXPECT_EQ(to_degrees(Direction::North), -90);
    EXPECT_EQ(to_degrees(Direction::East), 0);
    EXPECT_EQ(to_degrees(Direction::South), 90);
    EXPECT_EQ(to_degrees(Direction::West), -180);
}

TEST(DirectionTest, ToRadians)
{
    EXPECT_FLOAT_EQ(to_radians(Direction::North), -std::numbers::pi_v<float> / 2);
    EXPECT_FLOAT_EQ(to_radians(Direction::East), 0.0f);
    EXPECT_FLOAT_EQ(to_radians(Direction::South), std::numbers::pi_v<float> / 2);
    EXPECT_FLOAT_EQ(to_radians(Direction::West), -std::numbers::pi_v<float>);

    EXPECT_DOUBLE_EQ(to_radians<double>(Direction::North), -std::numbers::pi / 2);
    EXPECT_DOUBLE_EQ(to_radians<double>(Direction::East), 0.0);
    EXPECT_DOUBLE_EQ(to_radians<double>(Direction::South), std::numbers::pi / 2);
    EXPECT_DOUBLE_EQ(to_radians<double>(Direction::West), -std::numbers::pi);

    EXPECT_FLOAT_EQ(to_radians<Angle>(Direction::North).get(), -std::numbers::pi_v<float> / 2);
    EXPECT_FLOAT_EQ(to_radians<Angle>(Direction::East).get(), 0.0f);
    EXPECT_FLOAT_EQ(to_radians<Angle>(Direction::South).get(), std::numbers::pi_v<float> / 2);
    EXPECT_FLOAT_EQ(to_radians<Angle>(Direction::West).get(), -std::numbers::pi_v<float>);
}

TEST(DirectionTest, ToString)
{
    EXPECT_EQ(enum2str(Direction::North), "North");
    EXPECT_EQ(enum2str(Direction::East), "East");
    EXPECT_EQ(enum2str(Direction::South), "South");
    EXPECT_EQ(enum2str(Direction::West), "West");
}

}  // namespace micromouse::tests

REGISTER_TEST_FILE(direction_tests);
