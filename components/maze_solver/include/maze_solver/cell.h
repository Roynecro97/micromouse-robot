#ifndef MAZE_SOLVER_CELL_H
#define MAZE_SOLVER_CELL_H

#include <array>
#include <cstdint>
#include <utility>

namespace micromouse
{

enum class Walls : std::uint8_t
{
    North = 0x1,
    East = 0x2,
    South = 0x4,
    West = 0x8,
};

constexpr Walls operator|(Walls lhs, Walls rhs) noexcept
{
    return static_cast<Walls>(std::to_underlying(lhs) | std::to_underlying(rhs));
}
constexpr Walls operator&(Walls lhs, Walls rhs) noexcept
{
    return static_cast<Walls>(std::to_underlying(lhs) & std::to_underlying(rhs));
}
constexpr Walls operator^(Walls lhs, Walls rhs) noexcept
{
    return static_cast<Walls>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
}
constexpr Walls operator~(Walls walls) noexcept
{
    return static_cast<Walls>(~std::to_underlying(walls) & 0xF);
}

constexpr Walls &operator|=(Walls &lhs, Walls rhs) noexcept
{
    return lhs = lhs | rhs;
}
constexpr Walls &operator&=(Walls &lhs, Walls rhs) noexcept
{
    return lhs = lhs & rhs;
}
constexpr Walls &operator^=(Walls &lhs, Walls rhs) noexcept
{
    return lhs = lhs ^ rhs;
}

inline constexpr auto full_walls = Walls::North | Walls::East | Walls::South | Walls::West;
inline constexpr auto empty_walls = Walls{};

inline constexpr std::array all_walls = {Walls::North, Walls::East, Walls::South, Walls::West};

struct Cell
{
    Walls walls : 4;
    bool f3 : 1;
    bool f2 : 1;
    bool f1 : 1;
    bool visited : 1;

    constexpr Cell &operator|=(Walls rhs) noexcept
    {
        walls = walls | rhs;
        return *this;
    }
    constexpr Cell &operator&=(Walls rhs) noexcept
    {
        walls = walls & rhs;
        return *this;
    }
    constexpr Cell &operator^=(Walls rhs) noexcept
    {
        walls = walls ^ rhs;
        return *this;
    }
};

static_assert(sizeof(Cell) == 1);
static_assert(alignof(Cell) == alignof(std::uint8_t));
static_assert(alignof(Cell) == alignof(std::byte));

constexpr auto to_cell(Walls walls) noexcept
{
    return Cell{.walls{walls}, .f3{}, .f2{}, .f1{}, .visited{}};
}

constexpr const char *enum2str(Walls walls) noexcept
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    switch (walls & full_walls)
    {
    case Walls::North:
        return "North";
    case Walls::East:
        return "East";
    case Walls::North | Walls::East:
        return "North|East";
    case Walls::South:
        return "South";
    case Walls::North | Walls::South:
        return "North|South";
    case Walls::East | Walls::South:
        return "East|South";
    case Walls::North | Walls::East | Walls::South:
        return "North|East|South";
    case Walls::West:
        return "West";
    case Walls::North | Walls::West:
        return "North|West";
    case Walls::East | Walls::West:
        return "East|West";
    case Walls::North | Walls::East | Walls::West:
        return "North|East|West";
    case Walls::South | Walls::West:
        return "South|West";
    case Walls::North | Walls::South | Walls::West:
        return "North|South|West";
    case Walls::East | Walls::South | Walls::West:
        return "East|South|West";
    case Walls::North | Walls::East | Walls::South | Walls::West:
        return "North|East|South|West";
    default:  // case 0:
        return "<none>";
    }
#pragma GCC diagnostic pop
}

}  // namespace micromouse

#endif  // MAZE_SOLVER_CELL_H
