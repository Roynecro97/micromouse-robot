#ifndef MAIN_ALGORITHM_API_MOCK_H
#define MAIN_ALGORITHM_API_MOCK_H

#include <maze_solver/direction.h>
#include <misc_utils/physical_size.h>

#include "position.h"
#include "temp_map.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace micromouse
{

inline constexpr auto center_offset = unit_cast<meters>(35_mm);

template <bool use_center_offset = false>
constexpr Position translate_pos(std::size_t row, std::size_t col, Direction heading) noexcept
{
    auto center_pos = Position{
        XCoord{(col + 0.5f) * wall_length},
        YCoord{(row + 0.5f) * wall_length},
        to_radians<Angle>(heading),
    };
    if constexpr (use_center_offset)
    {
        switch (heading)
        {
        case Direction::North:
            center_pos.y += YCoord{center_offset};
            break;
        case Direction::East:
            center_pos.x -= XCoord{center_offset};
            break;
        case Direction::South:
            center_pos.y -= YCoord{center_offset};
            break;
        case Direction::West:
            center_pos.x += XCoord{center_offset};
            break;
        }
    }
    return center_pos;
}

constexpr Position convert(std::size_t row, std::size_t col, Direction heading) noexcept
{
    return translate_pos<true>(row, col, heading);
}

constexpr Position turn(std::size_t row, std::size_t col, Direction from, Direction to) noexcept
{
    auto res = translate_pos<true>(row, col, from);
    res.theta = to_radians<Angle>(to);
    return res;
}

inline constexpr std::array positions{
    convert(7, 0, Direction::East),  // Start point
    convert(7, 2, Direction::East),  turn(7, 2, Direction::East, Direction::North),
    convert(4, 2, Direction::North), turn(4, 2, Direction::North, Direction::West),
    convert(4, 0, Direction::West),  turn(4, 0, Direction::West, Direction::North),
    convert(3, 0, Direction::North), turn(3, 0, Direction::North, Direction::East),
    convert(3, 2, Direction::East),  turn(3, 2, Direction::East, Direction::North),
    convert(0, 2, Direction::North), turn(0, 2, Direction::North, Direction::East),
    convert(0, 5, Direction::East),  turn(0, 5, Direction::East, Direction::South),
    convert(2, 5, Direction::South), turn(2, 5, Direction::South, Direction::West),
    convert(2, 4, Direction::West),  turn(2, 4, Direction::West, Direction::South),
    convert(3, 4, Direction::South),  // Goal
};

class AlgorithmApi
{
public:
    std::optional<Position> get_next() noexcept
    {
        if (m_pos_index < positions.size())
        {
            Position pos = positions[m_pos_index];
            m_pos_index++;
            return pos;
        }
        else
        {
            return std::nullopt;
        }
    }

private:
    std::uint32_t m_pos_index = 0;
};

}  // namespace micromouse

#endif  // MAIN_ALGORITHM_API_MOCK_H
