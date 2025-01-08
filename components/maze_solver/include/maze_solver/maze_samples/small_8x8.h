#ifndef MAZE_SAMPLES_SMALL_8X8_H
#define MAZE_SAMPLES_SMALL_8X8_H

#include "../maze.h"

namespace micromouse::mazes
{
/**
 * +---+---+---+---+---+---+---+---+
 * |                           |   |
 * +   +---+   +---+---+   +   +   +
 * |   |   |   |       |   |       |
 * +   +   +   +   +   +   +   +   +
 * |       |   |   |       |   |   |
 * +---+   +   +---+   +---+   +   +
 * |           |       |       |   |
 * +   +---+---+   +   +   +---+   +
 * |           |       |       |   |
 * +---+   +   +---+---+---+   +   +
 * |       |       |           |   |
 * +   +---+   +   +   +---+   +   +
 * |           |   |   |           |
 * +---+---+   +   +   +---+---+   +
 * |           |               |   |
 * +---+---+---+---+---+---+---+---+
 */
inline const Maze<8, 8> small_8x8{std::array<std::array<Walls, 8>, 8>{{
    {Walls{0x9}, Walls{0x5}, Walls{0x1}, Walls{0x5}, Walls{0x5}, Walls{0x1}, Walls{0x3}, Walls{0xB}},
    {Walls{0xA}, Walls{0xB}, Walls{0xA}, Walls{0x9}, Walls{0x3}, Walls{0xA}, Walls{0x8}, Walls{0x2}},
    {Walls{0xC}, Walls{0x2}, Walls{0xA}, Walls{0xE}, Walls{0x8}, Walls{0x6}, Walls{0xA}, Walls{0xA}},
    {Walls{0x9}, Walls{0x4}, Walls{0x6}, Walls{0x9}, Walls{0x2}, Walls{0x9}, Walls{0x6}, Walls{0xA}},
    {Walls{0xC}, Walls{0x1}, Walls{0x3}, Walls{0xC}, Walls{0x6}, Walls{0xC}, Walls{0x3}, Walls{0xA}},
    {Walls{0x9}, Walls{0x6}, Walls{0x8}, Walls{0x3}, Walls{0x9}, Walls{0x5}, Walls{0x2}, Walls{0xA}},
    {Walls{0xC}, Walls{0x5}, Walls{0x2}, Walls{0xA}, Walls{0xA}, Walls{0xD}, Walls{0x4}, Walls{0x2}},
    {Walls{0xD}, Walls{0x5}, Walls{0x6}, Walls{0xC}, Walls{0x4}, Walls{0x5}, Walls{0x7}, Walls{0xE}},
}}};
}  // namespace micromouse::mazes

#endif  // MAZE_SAMPLES_SMALL_8X8_H
