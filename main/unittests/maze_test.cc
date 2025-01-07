#include "maze_solver/maze.h"

#include <maze_solver/cell.h>

#include "maze_solver_adapters.h"
#include "mem_equals.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <type_traits>

#include <hack.h>

using namespace std::literals;
using namespace testing;

namespace micromouse::tests
{

MATCHER_P(
    HasWalls,
    walls,
    "cell "s + (negation ? "doesn't have" : "has") + " all of " + ::testing::PrintToString(walls) + " set"
)
{
    return (arg & walls) == walls;
}

MATCHER_P(
    HasAnyWalls,
    walls,
    "cell "s + (negation ? "doesn't have" : "has") + " any of " + ::testing::PrintToString(walls) + " set"
)
{
    return (arg & walls) != empty_walls;
}

static_assert(Maze<8>::size() == 8 * 8);
static_assert(Maze<8>::height() == 8);
static_assert(Maze<8>::width() == 8);

static_assert(Maze<12, 10>::size() == 120);
static_assert(Maze<12, 10>::height() == 12);
static_assert(Maze<12, 10>::width() == 10);

namespace
{

// 3x3 is the smallest maze to have all external wall configurations and an empty cell
/**
 * +---+---+---+
 * |           |
 * +   +   +   +
 * |           |
 * +   +   +   +
 * |           |
 * +---+---+---+
 */
constexpr std::array empty_maze = {
    std::array{Walls{0x9}, Walls{0x1}, Walls{0x3}},
    std::array{Walls{0x8}, Walls{0x0}, Walls{0x2}},
    std::array{Walls{0xC}, Walls{0x4}, Walls{0x6}},
};

/**
 * +---+---+---+---+---+
 * |   |               |
 * +   +   +---+   +---+
 * |   |       |       |
 * +   +---+---+   +   +
 * |               |   |
 * +   +---+   +   +   +
 * |   |       |       |
 * +   +---+---+---+   +
 * |   |               |
 * +---+---+---+---+---+
 */
constexpr Walls c_base[5][5] = {
    {Walls{0xB}, Walls{0x9}, Walls{0x5}, Walls{0x1}, Walls{0x7}},
    {Walls{0xA}, Walls{0xC}, Walls{0x7}, Walls{0x8}, Walls{0x3}},
    {Walls{0x8}, Walls{0x5}, Walls{0x1}, Walls{0x2}, Walls{0xA}},
    {Walls{0xA}, Walls{0xD}, Walls{0x6}, Walls{0xC}, Walls{0x2}},
    {Walls{0xE}, Walls{0xD}, Walls{0x5}, Walls{0x5}, Walls{0x6}},
};

/**
 * +---+---+---+---+---+
 * |                   |
 * +---+---+   +---+---+
 * |       |   |   |   |
 * +   +   +   +   +   +
 * |   |   |       |   |
 * +   +---+   +---+   +
 * |               |   |
 * +   +   +---+   +   +
 * |   |               |
 * +---+---+---+---+---+
 */
constexpr std::array<std::array<Walls, 5>, 5> cpp_base = {{
    {Walls{0xD}, Walls{0x5}, Walls{0x1}, Walls{0x5}, Walls{0x7}},
    {Walls{0x9}, Walls{0x3}, Walls{0xA}, Walls{0xB}, Walls{0xB}},
    {Walls{0xA}, Walls{0xE}, Walls{0x8}, Walls{0x6}, Walls{0xA}},
    {Walls{0x8}, Walls{0x1}, Walls{0x4}, Walls{0x3}, Walls{0xA}},
    {Walls{0xE}, Walls{0xC}, Walls{0x5}, Walls{0x4}, Walls{0x6}},
}};

}  // namespace

template <typename T, std::size_t N>
static constexpr void recursive_copy(T (&dst)[N], const T (&src)[N])
{
    for (auto i = 0UZ; i < N; i++)
    {
        if constexpr (std::is_array_v<T>)
        {
            recursive_copy(dst[i], src[i]);
        }
        else
        {
            dst[i] = src[i];
        }
    }
}

template <typename T, std::size_t N>
static constexpr auto wrap(const T (&val)[N])
{
    struct
    {
        T v[N];
    } res;
    recursive_copy(res.v, val);
    return res;
}

class MazeTest : public ::testing::Test
{
protected:
    Maze<5> maze{c_base};
};

TEST_F(MazeTest, Constructors)
{
    using EmptyBase = std::remove_cvref_t<decltype(empty_maze)>;
    using EmptyMaze = Maze<extent_v<EmptyBase>, extent_v<EmptyBase, 1>>;
    ASSERT_THAT(EmptyMaze{}, MemoryEquals(empty_maze));
    ASSERT_THAT(maze, MemoryEquals(wrap(c_base)));
    ASSERT_THAT(Maze(cpp_base), MemoryEquals(cpp_base));

    static constexpr Walls maze_walls1[2][2] = {
        {Walls::North | Walls::West, Walls::North | Walls::East},
        {Walls::South | Walls::West, Walls::East | Walls::South},
    };

    static constexpr Cell maze_walls2[2][2] = {
        {to_cell(Walls::North | Walls::West), to_cell(Walls::North | Walls::East)},
        {to_cell(Walls::South | Walls::West), to_cell(Walls::East | Walls::South)},
    };

    static constexpr std::array<std::array<Walls, 2>, 2> maze_walls3 = {{
        {Walls::North | Walls::West, Walls::North | Walls::East},
        {Walls::South | Walls::West, Walls::East | Walls::South},
    }};

    static constexpr std::array<std::array<Cell, 2>, 2> maze_walls4 = {{
        {to_cell(Walls::North | Walls::West), to_cell(Walls::North | Walls::East)},
        {to_cell(Walls::South | Walls::West), to_cell(Walls::East | Walls::South)},
    }};

    static constexpr Maze m;
    static constexpr Maze<2> m_2;
    static constexpr Maze<1> m_1;
    static_assert(m_1[0, 0] == full_walls);

    static constexpr Maze<2> m1{maze_walls1};
    static constexpr Maze<2> m2{maze_walls2};
    static constexpr Maze<2> m3{maze_walls3};
    static constexpr Maze<2> m4{maze_walls4};

    static constexpr Maze dm1{maze_walls1};
    static constexpr Maze dm2{maze_walls2};
    static constexpr Maze dm3{maze_walls3};
    static constexpr Maze dm4{maze_walls4};
}

TEST_F(MazeTest, ConstIndexOperator)
{
    const auto &cm = maze;
    for (auto row = 0UZ; row < cm.height(); row++)
    {
        for (auto col = 0UZ; col < cm.width(); col++)
        {
            decltype(auto) maze_walls = cm[row, col];
            static_assert(std::is_same_v<decltype(maze_walls), Walls>);
            EXPECT_EQ(maze_walls, c_base[row][col]) << " row = " << row << ", col = " << col;
        }
    }
}

TEST_F(MazeTest, AddWalls)
{
    const auto &cm = maze;

    ASSERT_THAT((cm[1, 1]), Not(HasAnyWalls(Walls::North))) << " bad assumption in the test";
    maze.add_walls(1, 1, Walls::North);
    EXPECT_THAT((cm[1, 1]), HasWalls(Walls::North));
    EXPECT_THAT((cm[0, 1]), HasWalls(Walls::South));

    ASSERT_THAT((cm[1, 3]), Not(HasAnyWalls(Walls::East))) << " bad assumption in the test";
    maze.add_walls(1, 3, Walls::East);
    EXPECT_THAT((cm[1, 3]), HasWalls(Walls::East));
    EXPECT_THAT((cm[1, 4]), HasWalls(Walls::West));

    ASSERT_THAT((cm[2, 3]), Not(HasAnyWalls(Walls::South))) << " bad assumption in the test";
    maze.add_walls(2, 3, Walls::South);
    EXPECT_THAT((cm[2, 3]), HasWalls(Walls::South));
    EXPECT_THAT((cm[3, 3]), HasWalls(Walls::North));

    ASSERT_THAT((cm[3, 4]), Not(HasAnyWalls(Walls::West))) << " bad assumption in the test";
    maze.add_walls(3, 4, Walls::West);
    EXPECT_THAT((cm[3, 4]), HasWalls(Walls::West));
    EXPECT_THAT((cm[3, 3]), HasWalls(Walls::East));
}

TEST_F(MazeTest, RemoveWalls)
{
    const auto &cm = maze;

    ASSERT_THAT((cm[1, 2]), HasWalls(Walls::North)) << " bad assumption in the test";
    maze.remove_walls(1, 2, Walls::North);
    EXPECT_THAT((cm[1, 2]), Not(HasAnyWalls(Walls::North)));
    EXPECT_THAT((cm[0, 2]), Not(HasAnyWalls(Walls::South)));

    ASSERT_THAT((cm[2, 3]), HasWalls(Walls::East)) << " bad assumption in the test";
    maze.remove_walls(2, 3, Walls::East);
    EXPECT_THAT((cm[2, 3]), Not(HasAnyWalls(Walls::East)));
    EXPECT_THAT((cm[2, 4]), Not(HasAnyWalls(Walls::West)));

    ASSERT_THAT((cm[2, 1]), HasWalls(Walls::South)) << " bad assumption in the test";
    maze.remove_walls(2, 1, Walls::South);
    EXPECT_THAT((cm[2, 1]), Not(HasAnyWalls(Walls::South)));
    EXPECT_THAT((cm[1, 1]), Not(HasAnyWalls(Walls::North)));

    ASSERT_THAT((cm[4, 1]), HasWalls(Walls::West)) << " bad assumption in the test";
    maze.remove_walls(4, 1, Walls::West);
    EXPECT_THAT((cm[4, 1]), Not(HasAnyWalls(Walls::West)));
    EXPECT_THAT((cm[4, 0]), Not(HasAnyWalls(Walls::East)));
}

TEST_F(MazeTest, AddRemoveWalls)
{
    // Use checkered patterns to add/remove walls to make sure the maze adds/removes walls properly when needed.
    // The bounding walls are always there so there's no need to remove *all* of them.
    const auto &cm = maze;

    for (auto row = 0UZ; row < cm.height(); row++)
    {
        for (auto col = (row & 1UZ); col < cm.width(); col += 2)
        {
            maze.add_walls(row, col, full_walls);
        }
    }

    for (auto row = 0UZ; row < cm.height(); row++)
    {
        for (auto col = 0UZ; col < cm.width(); col++)
        {
            EXPECT_THAT((cm[row, col]), HasWalls(full_walls)) << " row = " << row << ", col = " << col;
        }
    }

    for (auto row = 0UZ; row < cm.height(); row++)
    {
        for (auto col = 1UZ - (row & 1UZ); col < cm.width(); col++)
        {
            maze.remove_walls(row, col, full_walls);
        }
    }

    for (auto row = 0UZ; row < cm.height(); row++)
    {
        for (auto col = 0UZ; col < cm.width(); col++)
        {
            auto expected_walls = empty_walls;
            if (row == 0)
            {
                expected_walls |= Walls::North;
            }
            else if (row == cm.height() - 1)
            {
                expected_walls |= Walls::South;
            }
            if (col == 0)
            {
                expected_walls |= Walls::West;
            }
            else if (col == cm.width() - 1)
            {
                expected_walls |= Walls::East;
            }

            ASSERT_THAT((cm[row, col]), AllOf(HasWalls(expected_walls), Not(HasAnyWalls(~expected_walls))))
                << " row = " << row << ", col = " << col;
        }
    }
}

TEST_F(MazeTest, NonConstIndexOperatorGet)
{
    for (auto row = 0UZ; row < maze.height(); row++)
    {
        for (auto col = 0UZ; col < maze.width(); col++)
        {
            EXPECT_EQ((maze[row, col]), c_base[row][col]) << " row = " << row << ", col = " << col;
        }
    }
}

TEST_F(MazeTest, NonConstIndexOperatorSet)
{
    ASSERT_THAT(cpp_base, Not(MemoryEquals(wrap(c_base)))) << " mazes must be different";

    ASSERT_NE((maze[1, 3]), Walls::East) << " bad assumption in the test";
    maze[1, 3] = Walls::East;
    EXPECT_EQ((maze[1, 3]), Walls::East);
    EXPECT_THAT((maze[0, 3]), Not(HasAnyWalls(Walls::South)));
    EXPECT_THAT((maze[1, 4]), HasWalls(Walls::West));
    EXPECT_THAT((maze[2, 3]), Not(HasAnyWalls(Walls::North)));
    EXPECT_THAT((maze[1, 2]), Not(HasAnyWalls(Walls::East)));

    ASSERT_NE((maze[1, 2]), Walls::East | Walls::West) << " bad assumption in the test";
    maze[1, 2] = Walls::East | Walls::West;
    EXPECT_EQ((maze[1, 2]), Walls::East | Walls::West);
    EXPECT_THAT((maze[0, 2]), Not(HasAnyWalls(Walls::South)));
    EXPECT_THAT((maze[1, 3]), HasWalls(Walls::West));
    EXPECT_THAT((maze[2, 2]), Not(HasAnyWalls(Walls::North)));
    EXPECT_THAT((maze[1, 1]), HasWalls(Walls::East));

    maze[1, 2] = Walls::North | Walls::South;
    EXPECT_EQ((maze[1, 2]), Walls::North | Walls::South);
    EXPECT_THAT((maze[0, 2]), HasWalls(Walls::South));
    EXPECT_THAT((maze[1, 3]), Not(HasAnyWalls(Walls::West)));
    EXPECT_THAT((maze[2, 2]), HasWalls(Walls::North));
    EXPECT_THAT((maze[1, 1]), Not(HasAnyWalls(Walls::East)));

    maze[0, 1] = Walls::East;
    EXPECT_EQ((maze[0, 1]), Walls::North | Walls::East);
    EXPECT_THAT((maze[0, 2]), HasWalls(Walls::West));
    EXPECT_THAT((maze[1, 1]), Not(HasAnyWalls(Walls::North)));
    EXPECT_THAT((maze[0, 0]), Not(HasAnyWalls(Walls::East)));

    maze[3, 0] = Walls::East;
    EXPECT_EQ((maze[3, 0]), Walls::East | Walls::West);
    EXPECT_THAT((maze[2, 0]), Not(HasAnyWalls(Walls::South)));
    EXPECT_THAT((maze[3, 1]), HasWalls(Walls::West));
    EXPECT_THAT((maze[4, 0]), Not(HasAnyWalls(Walls::North)));

    maze[4, 1] = Walls::East;
    EXPECT_EQ((maze[4, 1]), Walls::East | Walls::South);
    EXPECT_THAT((maze[3, 1]), Not(HasAnyWalls(Walls::South)));
    EXPECT_THAT((maze[4, 2]), HasWalls(Walls::West));
    EXPECT_THAT((maze[4, 0]), Not(HasAnyWalls(Walls::East)));

    maze[3, 4] = Walls::South;
    EXPECT_EQ((maze[3, 4]), Walls::East | Walls::South);
    EXPECT_THAT((maze[2, 4]), Not(HasAnyWalls(Walls::South)));
    EXPECT_THAT((maze[4, 4]), HasWalls(Walls::North));
    EXPECT_THAT((maze[3, 3]), Not(HasAnyWalls(Walls::East)));
}

TEST_F(MazeTest, NonConstIndexOperatorSetAll)
{
    ASSERT_THAT(cpp_base, Not(MemoryEquals(wrap(c_base)))) << " mazes must be different";

    for (auto row = 0UZ; row < maze.height(); row++)
    {
        for (auto col = 0UZ; col < maze.width(); col++)
        {
            maze[row, col] = cpp_base[row][col];
        }
    }
    for (auto row = 0UZ; row < maze.height(); row++)
    {
        for (auto col = 0UZ; col < maze.width(); col++)
        {
            EXPECT_EQ((maze[row, col]), cpp_base[row][col]) << " row = " << row << ", col = " << col;
        }
    }
}

}  // namespace micromouse::tests

REGISTER_TEST_FILE(maze_tests);
