#ifndef MAIN_TEMP_MAP_H
#define MAIN_TEMP_MAP_H

#include <misc_utils/physical_size.h>

#include "position.h"
#include "segment.h"

#include <array>

namespace micromouse
{

inline constexpr auto wall_length = unit_cast<meters>(18_cm).count();

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
inline const std::array maze_map = {
    // Bounding Box
    /**
     * +---+---+---+---+---+---+---+---+
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{0.0f, 0.0f}, Eigen::Vector2f{8 * wall_length, 0.0f}},
    Segment{Eigen::Vector2f{0.0f, 0.0f}, Eigen::Vector2f{0.0f, 8 * wall_length}},
    Segment{Eigen::Vector2f{8 * wall_length, 0.0f}, Eigen::Vector2f{8 * wall_length, 8 * wall_length}},
    Segment{Eigen::Vector2f{0.0f, 8 * wall_length}, Eigen::Vector2f{8 * wall_length, 8 * wall_length}},

    // Inner Walls
    /**
     * +---+---+---+---+---+---+---+---+
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+---+   +   +   +   +   +   +
     * |                               |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{0 * wall_length, 3 * wall_length}, Eigen::Vector2f{1 * wall_length, 3 * wall_length}},
    Segment{Eigen::Vector2f{0 * wall_length, 5 * wall_length}, Eigen::Vector2f{1 * wall_length, 5 * wall_length}},
    Segment{Eigen::Vector2f{0 * wall_length, 7 * wall_length}, Eigen::Vector2f{2 * wall_length, 7 * wall_length}},

    /**
     * +---+---+---+---+---+---+---+---+
     * |                               |
     * +   +---+   +   +   +   +   +   +
     * |   |   |                       |
     * +   +   +   +   +   +   +   +   +
     * |       |                       |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{1 * wall_length, 1 * wall_length}, Eigen::Vector2f{1 * wall_length, 2 * wall_length}},
    Segment{Eigen::Vector2f{1 * wall_length, 1 * wall_length}, Eigen::Vector2f{2 * wall_length, 1 * wall_length}},
    Segment{Eigen::Vector2f{2 * wall_length, 1 * wall_length}, Eigen::Vector2f{2 * wall_length, 3 * wall_length}},

    /**
     * +---+---+---+---+---+---+---+---+
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |       |                       |
     * +   +---+   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{1 * wall_length, 6 * wall_length}, Eigen::Vector2f{2 * wall_length, 6 * wall_length}},
    Segment{Eigen::Vector2f{2 * wall_length, 5 * wall_length}, Eigen::Vector2f{2 * wall_length, 6 * wall_length}},

    /**
     * +---+---+---+---+---+---+---+---+
     * |                               |
     * +   +   +   +---+---+   +   +   +
     * |           |       |   |       |
     * +   +   +   +   +   +   +   +   +
     * |           |   |       |       |
     * +   +   +   +---+   +---+   +   +
     * |           |       |           |
     * +   +---+---+   +   +   +   +   +
     * |           |       |           |
     * +   +   +   +---+---+---+   +   +
     * |               |               |
     * +   +   +   +   +   +   +   +   +
     * |               |               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{1 * wall_length, 4 * wall_length}, Eigen::Vector2f{3 * wall_length, 4 * wall_length}},
    Segment{Eigen::Vector2f{3 * wall_length, 1 * wall_length}, Eigen::Vector2f{3 * wall_length, 5 * wall_length}},
    Segment{Eigen::Vector2f{3 * wall_length, 1 * wall_length}, Eigen::Vector2f{5 * wall_length, 1 * wall_length}},
    Segment{Eigen::Vector2f{5 * wall_length, 1 * wall_length}, Eigen::Vector2f{5 * wall_length, 2 * wall_length}},
    Segment{Eigen::Vector2f{3 * wall_length, 3 * wall_length}, Eigen::Vector2f{4 * wall_length, 3 * wall_length}},
    Segment{Eigen::Vector2f{4 * wall_length, 2 * wall_length}, Eigen::Vector2f{4 * wall_length, 3 * wall_length}},
    Segment{Eigen::Vector2f{3 * wall_length, 5 * wall_length}, Eigen::Vector2f{6 * wall_length, 5 * wall_length}},
    Segment{Eigen::Vector2f{4 * wall_length, 5 * wall_length}, Eigen::Vector2f{4 * wall_length, 7 * wall_length}},
    Segment{Eigen::Vector2f{5 * wall_length, 3 * wall_length}, Eigen::Vector2f{5 * wall_length, 5 * wall_length}},
    Segment{Eigen::Vector2f{5 * wall_length, 3 * wall_length}, Eigen::Vector2f{6 * wall_length, 3 * wall_length}},
    Segment{Eigen::Vector2f{6 * wall_length, 1 * wall_length}, Eigen::Vector2f{6 * wall_length, 3 * wall_length}},

    /**
     * +---+---+---+---+---+---+---+---+
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |           |                   |
     * +   +   +   +   +   +   +   +   +
     * |           |                   |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{3 * wall_length, 6 * wall_length}, Eigen::Vector2f{3 * wall_length, 8 * wall_length}},

    /**
     * +---+---+---+---+---+---+---+---+
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +---+   +   +
     * |                   |           |
     * +   +   +   +   +   +---+---+   +
     * |                           |   |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{5 * wall_length, 6 * wall_length}, Eigen::Vector2f{6 * wall_length, 6 * wall_length}},
    Segment{Eigen::Vector2f{5 * wall_length, 6 * wall_length}, Eigen::Vector2f{5 * wall_length, 7 * wall_length}},
    Segment{Eigen::Vector2f{5 * wall_length, 7 * wall_length}, Eigen::Vector2f{7 * wall_length, 7 * wall_length}},
    Segment{Eigen::Vector2f{7 * wall_length, 7 * wall_length}, Eigen::Vector2f{7 * wall_length, 8 * wall_length}},

    /**
     * +---+---+---+---+---+---+---+---+
     * |                           |   |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{7 * wall_length, 0 * wall_length}, Eigen::Vector2f{7 * wall_length, 1 * wall_length}},

    /**
     * +---+---+---+---+---+---+---+---+
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                           |   |
     * +   +   +   +   +   +   +   +   +
     * |                           |   |
     * +   +   +   +   +   +   +---+   +
     * |                           |   |
     * +   +   +   +   +   +   +   +   +
     * |                           |   |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +   +   +   +   +   +   +   +   +
     * |                               |
     * +---+---+---+---+---+---+---+---+
     */
    Segment{Eigen::Vector2f{6 * wall_length, 4 * wall_length}, Eigen::Vector2f{7 * wall_length, 4 * wall_length}},
    Segment{Eigen::Vector2f{7 * wall_length, 2 * wall_length}, Eigen::Vector2f{7 * wall_length, 6 * wall_length}},
};

}  // namespace micromouse

#endif  // MAIN_TEMP_MAP_H
