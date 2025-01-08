#include "maze_solver/cell.h"

#include <maze_solver/direction.h>

#include "maze_solver_adapters.h"
#include "mem_equals.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <bit>
#include <cstring>
#include <string>
#include <type_traits>

#include <hack.h>

using namespace std::literals;

namespace micromouse::tests
{

namespace
{
constexpr std::array all_wall_combinations = {
    Walls{},
    Walls::North,
    Walls::East,
    Walls::North | Walls::East,
    Walls::South,
    Walls::North | Walls::South,
    Walls::East | Walls::South,
    Walls::North | Walls::East | Walls::South,
    Walls::West,
    Walls::North | Walls::West,
    Walls::East | Walls::West,
    Walls::North | Walls::East | Walls::West,
    Walls::South | Walls::West,
    Walls::North | Walls::South | Walls::West,
    Walls::East | Walls::South | Walls::West,
    Walls::North | Walls::East | Walls::South | Walls::West,
};
}  // namespace

template <Direction D>
consteval std::byte to_byte() noexcept
{
    static_assert(std::to_underlying(D) >= 0 && std::to_underlying(D) <= 0xFF);
    return std::byte(std::to_underlying(D));
}

TEST(WallTest, DirectionMemoryMatches)
{
    EXPECT_THAT(Walls::North, MemoryEquals(to_byte<Direction::North>()));
    EXPECT_THAT(Walls::East, MemoryEquals(to_byte<Direction::East>()));
    EXPECT_THAT(Walls::South, MemoryEquals(to_byte<Direction::South>()));
    EXPECT_THAT(Walls::West, MemoryEquals(to_byte<Direction::West>()));
}

TEST(WallTest, Invert)
{
    EXPECT_EQ(~Walls::North, Walls::East | Walls::South | Walls::West);
    EXPECT_EQ(~Walls::East, Walls::North | Walls::South | Walls::West);
    EXPECT_EQ(~Walls::South, Walls::North | Walls::East | Walls::West);
    EXPECT_EQ(~Walls::West, Walls::North | Walls::East | Walls::South);
}

TEST(WallTest, EmptyAndFullWalls)
{
    for (const auto walls : all_wall_combinations)
    {
        EXPECT_EQ(empty_walls & walls, empty_walls);
        EXPECT_EQ(empty_walls | walls, walls);
        EXPECT_EQ(full_walls & walls, walls);
        EXPECT_EQ(full_walls | walls, full_walls);

        const auto invalid_bits = Walls{0x50};
        const auto bad_walls = walls | invalid_bits;
        EXPECT_EQ(empty_walls & bad_walls, empty_walls);
        EXPECT_EQ(empty_walls | bad_walls, bad_walls);
        EXPECT_EQ(full_walls & bad_walls, walls);
        EXPECT_EQ(full_walls | bad_walls, full_walls | invalid_bits);
    }
}

TEST(CellTest, FactoryFunction)
{
    for (const auto walls : all_wall_combinations)
    {
        const auto cell = to_cell(walls);
        EXPECT_EQ(cell.walls, walls);
        EXPECT_FALSE(cell.f3);
        EXPECT_FALSE(cell.f1);
        EXPECT_FALSE(cell.f2);
        EXPECT_FALSE(cell.visited);
    }
}

TEST(CellTest, WallMemoryMatches)
{
    for (const auto walls : all_wall_combinations)
    {
        EXPECT_EQ(std::bit_cast<std::byte>(to_cell(walls)), std::bit_cast<std::byte>(walls));
        EXPECT_THAT(to_cell(walls), MemoryEquals(walls));
    }

    EXPECT_THAT(to_cell(Walls::North), MemoryEquals(to_byte<Direction::North>()));
    EXPECT_THAT(to_cell(Walls::East), MemoryEquals(to_byte<Direction::East>()));
    EXPECT_THAT(to_cell(Walls::South), MemoryEquals(to_byte<Direction::South>()));
    EXPECT_THAT(to_cell(Walls::West), MemoryEquals(to_byte<Direction::West>()));
}

}  // namespace micromouse::tests

REGISTER_TEST_FILE(cell_tests);
