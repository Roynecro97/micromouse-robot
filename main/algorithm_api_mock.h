#include <array>
#include <cstdint>
#include <optional>

struct motor_distance
{
    static constexpr uint16_t next_cell_wheel_distance_mm = 180;
    static constexpr uint16_t turn_90_wheel_distance_mm = 170; // When only one wheel turns.

    float left_distance;
    float right_distance;
};

enum class heading
{
    north,
    west,
    south,
    east,
};

class command
{
public:
    enum type {
        stop,
        forward_cell_drive,
        backward_cell_drive,
        turn_left_90,
        turn_right_90,
    };

    command(): m_t(stop) {};
    command(type t): m_t(t) {};

    constexpr motor_distance to_distance() const noexcept
    {
        motor_distance result;
        switch (m_t)
        {
        case command::forward_cell_drive:
            result.left_distance = motor_distance::next_cell_wheel_distance_mm;
            result.right_distance = motor_distance::next_cell_wheel_distance_mm;
            break;
        case command::backward_cell_drive:
            result.left_distance = -motor_distance::next_cell_wheel_distance_mm;
            result.right_distance = -motor_distance::next_cell_wheel_distance_mm;
            break;
        case command::turn_left_90:
            result.left_distance = 0;
            result.right_distance = motor_distance::turn_90_wheel_distance_mm;
            break;
        case command::turn_right_90:
            result.left_distance = motor_distance::turn_90_wheel_distance_mm;
            result.right_distance = 0;
            break;
        default:
            break;
        }

        return result;
    }

private:
    type m_t;
};

struct cell_coordinate
{
    uint16_t x = 0;
    uint16_t y = 0;

    // Assuming the destination is one of the 4 cells connected to source.
    static std::pair<command, heading> to_command(const cell_coordinate& source, const heading& current_heading, const cell_coordinate& destination)
    {
        std::pair<command, heading> result;
        if (source.x > destination.x)
        {
            // Go to the cell on the west
            switch (current_heading)
            {
            case heading::north:
                result.first = command{command::type::turn_left_90};
                result.second = heading::west;
                break;
            case heading::south:
                result.first = command{command::type::turn_right_90};
                result.second = heading::west;
                break;
            case heading::west:
                result.first = command{command::type::forward_cell_drive};
                result.second = heading::west;
                break;
            case heading::east:
                result.first = command{command::type::backward_cell_drive};
                result.second = heading::east;
                break;
            default:
                break;
            }
        }
        else if (source.x < destination.x)
        {
            // Go to the cell on the east
            switch (current_heading)
            {
            case heading::north:
                result.first = command{command::type::turn_right_90};
                result.second = heading::east;
                break;
            case heading::south:
                result.first = command{command::type::turn_left_90};
                result.second = heading::east;
                break;
            case heading::west:
                result.first = command{command::type::backward_cell_drive};
                result.second = heading::west;
                break;
            case heading::east:
                result.first = command{command::type::forward_cell_drive};
                result.second = heading::east;
                break;
            default:
                break;
            }
        }
        else if (source.y > destination.y)
        {
            // Go to the cell on the south
            switch (current_heading)
            {
            case heading::north:
                result.first = command{command::type::backward_cell_drive};
                result.second = heading::north;
                break;
            case heading::south:
                result.first = command{command::type::forward_cell_drive};
                result.second = heading::south;
                break;
            case heading::west:
                result.first = command{command::type::turn_right_90};
                result.second = heading::south;
                break;
            case heading::east:
                result.first = command{command::type::turn_left_90};
                result.second = heading::south;
                break;
            default:
                break;
            }
        }
        else if (source.y < destination.y)
        {
            // Go to the cell on the north
            switch (current_heading)
            {
            case heading::north:
                result.first = command{command::type::forward_cell_drive};
                result.second = heading::north;
                break;
            case heading::south:
                result.first = command{command::type::backward_cell_drive};
                result.second = heading::south;
                break;
            case heading::west:
                result.first = command{command::type::turn_right_90};
                result.second = heading::north;
                break;
            case heading::east:
                result.first = command{command::type::turn_left_90};
                result.second = heading::north;
                break;
            default:
                break;
            }
        }

        return result;
    }
};

static cell_coordinate end_coordinate{};

std::array cells{
    cell_coordinate{0, 1},
    cell_coordinate{1, 1},
    cell_coordinate{1, 0},
    cell_coordinate{0, 0},
};

class algorithm_api
{
    uint32_t i = 0;

public:
    std::optional<cell_coordinate> get_next()
    {
        if (i < cells.size())
        {
            const auto& ret = cells[i];
            i++;
            return ret;
        }

        return std::nullopt;
    }
};
