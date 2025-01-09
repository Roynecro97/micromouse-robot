#ifndef MAIN_DISTANCE_SENSOR_H
#define MAIN_DISTANCE_SENSOR_H

#include <misc_utils/physical_size.h>

#include "average_filter.h"
#include "position.h"
#include "segment.h"

#include <Eigen/Core>

#include <array>
#include <span>
#include <vector>

class TwoWire;
class QWIICMUX;
class SFEVL53L1X;

enum Vl53l1cdTimingBudget : uint16_t
{
    VL53L1CD_TimingBudget_15ms = 15,
    VL53L1CD_TimingBudget_20ms = 20,
    VL53L1CD_TimingBudget_33ms = 33,
    VL53L1CD_TimingBudget_50ms = 50,
    VL53L1CD_TimingBudget_100ms = 100,
    VL53L1CD_TimingBudget_200ms = 200,
    VL53L1CD_TimingBudget_500ms = 500,
};

class DistanceSensors
{
public:
    static constexpr auto sensor_count = 5;
    static constexpr auto avg_filter_size = 1;
    using Measurements = Eigen::Vector<float, sensor_count>;
    using Jacobian = Eigen::Matrix<float, sensor_count, 3>;

    DistanceSensors(const DistanceSensors &) = delete;
    DistanceSensors(DistanceSensors &&) = delete;
    DistanceSensors &operator=(const DistanceSensors &) = delete;
    DistanceSensors &operator=(DistanceSensors &&) = delete;
    ~DistanceSensors() noexcept = default;

    static DistanceSensors &get_instance() noexcept;
    void init(TwoWire &i2c, Vl53l1cdTimingBudget timing_budget = VL53L1CD_TimingBudget_20ms) noexcept;

    std::array<micromouse::meters, sensor_count> read_all() noexcept;

    std::pair<Measurements, Jacobian> predict(
        const micromouse::Position &pos,
        const std::span<const micromouse::Segment> &maze_map
    ) noexcept;

private:
    constexpr DistanceSensors(QWIICMUX &mux, SFEVL53L1X &current_distance_sensor) noexcept
        : m_mux{mux}
        , m_current_distance_sensor{current_distance_sensor}
    {
    }

    struct Sensor
    {
        explicit constexpr Sensor(std::uint8_t port_number) noexcept : port{port_number} {}

        constexpr auto get_distance() const noexcept
        {
            return unit_cast<micromouse::meters>(micromouse::millimeters{distance});
        }

        avg_filter<std::uint16_t, avg_filter_size, int> distance;
        const std::uint8_t port;
    };

    QWIICMUX &m_mux;
    SFEVL53L1X &m_current_distance_sensor;
    std::array<Sensor, sensor_count> m_sensors{Sensor{1}, Sensor{2}, Sensor{3}, Sensor{4}, Sensor{5}};
};

#endif  // MAIN_DISTANCE_SENSOR_H
