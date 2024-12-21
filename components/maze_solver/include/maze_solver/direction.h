#ifndef MAZE_SOLVER_DIRECTION_H
#define MAZE_SOLVER_DIRECTION_H

#include <misc_utils/typing_utils.h>

#include <array>
#include <limits>
#include <numbers>

namespace micromouse
{

enum class RelativeDirection
{
    Front,
    Back,
    Left,
    Right,
};

/**
 * @brief Invert the direction (left <-> right; front <-> back).
 *
 * @param rd The relative direction to invert.
 * @return The inverted direction.
 */
constexpr auto invert(RelativeDirection rd) noexcept
{
    switch (rd)
    {
    case RelativeDirection::Front:
        return RelativeDirection::Back;
    case RelativeDirection::Back:
        return RelativeDirection::Front;
    case RelativeDirection::Left:
        return RelativeDirection::Right;
    case RelativeDirection::Right:
        return RelativeDirection::Left;
    }
    return rd;
}

constexpr auto enum2str(RelativeDirection rd) noexcept
{
    switch (rd)
    {
    case RelativeDirection::Front:
        return "Front";
    case RelativeDirection::Back:
        return "Back";
    case RelativeDirection::Left:
        return "Left";
    case RelativeDirection::Right:
        return "Right";
    default:
        return "<invalid>";
    }
}

inline constexpr std::array relative_directions{
    RelativeDirection::Front,
    RelativeDirection::Back,
    RelativeDirection::Left,
    RelativeDirection::Right,
};

/**
 * @brief A cardinal direction.
 * @note Values match the corresponding Wall enum values except multiple Directions cannot be mixed.
 * @see Wall
 */
enum class Direction
{
    North = 0x1,
    East = 0x2,
    South = 0x4,
    West = 0x8,
};

/**
 * @brief Return the direction that is the result of turning left (90 degrees counter-clockwise).
 *
 * @param d The cardinal direction to turn from.
 * @return The result of turning left.
 */
constexpr auto turn_left(Direction d) noexcept
{
    switch (d)
    {
    case Direction::North:
        return Direction::West;
    case Direction::East:
        return Direction::North;
    case Direction::South:
        return Direction::East;
    case Direction::West:
        return Direction::South;
    }
    return d;
}

/**
 * @brief Calculate the direction that is the result of turning right (90 degrees clockwise).
 *
 * @param d The cardinal direction to turn from.
 * @return The result of turning right.
 */
constexpr auto turn_right(Direction d) noexcept
{
    switch (d)
    {
    case Direction::North:
        return Direction::East;
    case Direction::East:
        return Direction::South;
    case Direction::South:
        return Direction::West;
    case Direction::West:
        return Direction::North;
    }
    return d;
}

/**
 * @brief Calculate the direction that is the result of turning back (180 degrees).
 *
 * @param d The cardinal direction to turn from.
 * @return The result of turning back.
 */
constexpr auto turn_back(Direction d) noexcept
{
    switch (d)
    {
    case Direction::North:
        return Direction::South;
    case Direction::East:
        return Direction::West;
    case Direction::South:
        return Direction::North;
    case Direction::West:
        return Direction::East;
    }
    return d;
}

/**
 * @brief Calculate the direction that is the result of turning in the provided relative direction.
 *
 * @param d The cardinal direction to turn from.
 * @param rel The direction to turn to.
 * @return The result of the turn.
 */
constexpr auto turn(Direction d, RelativeDirection rel) noexcept
{
    switch (rel)
    {
    case RelativeDirection::Front:
        return d;
    case RelativeDirection::Back:
        return turn_back(d);
    case RelativeDirection::Left:
        return turn_left(d);
    case RelativeDirection::Right:
        return turn_right(d);
    }
    return d;
}

/**
 * @brief Get the rotation degrees. East is 0 deg, degrees increase clockwise.
 *
 * @param d A cardinal direction.
 * @return Clockwise-rotation degrees from the East direction.
 */
constexpr auto to_degrees(Direction d) noexcept
{
    switch (d)
    {
    case Direction::North:
        return -90;
    case Direction::East:
        return 0;
    case Direction::South:
        return 90;
    case Direction::West:
        return -180;
    }
    return 720;  // impossible value, should pop out
}

/**
 * @brief Get the rotation angle. East is 0, degrees increase clockwise.
 *
 * @param d A cardinal direction.
 * @return Clockwise-rotation angle (in radians) from the East direction.
 */
template <ExtendedFloatingPoint F = float>
constexpr auto to_radians(Direction d) noexcept
{
    using float_type = make_floating_point<F>;
    switch (d)
    {
    case Direction::North:
        return F(-std::numbers::pi_v<float_type> / 2);
    case Direction::East:
        return F(0.0f);
    case Direction::South:
        return F(std::numbers::pi_v<float_type> / 2);
    case Direction::West:
        return F(-std::numbers::pi_v<float_type>);
    default:
        return F(std::numeric_limits<float_type>::quiet_NaN());
    }
}

constexpr auto enum2str(Direction d) noexcept
{
    switch (d)
    {
    case Direction::North:
        return "North";
    case Direction::East:
        return "East";
    case Direction::South:
        return "South";
    case Direction::West:
        return "West";
    default:
        return "<invalid>";
    }
}

inline constexpr std::array primary_directions{
    Direction::North,
    Direction::East,
    Direction::South,
    Direction::West,
};

}  // namespace micromouse

#endif  // MAZE_SOLVER_DIRECTION_H
