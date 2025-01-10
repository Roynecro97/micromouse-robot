#include "distance_sensor.h"

#include <misc_utils/angle.h>

#include "led_loop_utils.h"

#include <Eigen/Dense>

#include <limits>
#include <numbers>
#include <ranges>

#include <Arduino.h>
#include <SparkFun_I2C_Mux_Arduino_Library.h>
#include <SparkFun_VL53L1X.h>
#include <esp_log.h>

using namespace micromouse;

static constexpr auto max_sensor_range = 0.15_m /* 1.3_m / 4 */;
static constexpr auto max_predict_range = max_sensor_range /* 3.0_m */;

// Sensors disposition relative to the center of rotation in meters.
static constexpr std::array sensor_disposition{
    std::pair(XCoord{62.0e-3f}, YCoord{54.0e-3f}),
    std::pair(XCoord{90.0e-3f}, YCoord{40.0e-3f}),
    std::pair(XCoord{94.0e-3f}, YCoord{0.0f}),
    std::pair(XCoord{92.0e-3f}, YCoord{-39.0e-3f}),
    std::pair(XCoord{67.0e-3f}, YCoord{-54.0e-3f}),
};
static_assert(sensor_disposition.size() == DistanceSensors::sensor_count);

// Sensor angles relative to the robot in radians.
static constexpr std::array sensor_angles{
    Angle{std::numbers::pi_v<float> / 2},
    Angle{0.703765f /* std::numbers::pi_v<float> / 4 */},
    Angle{0.0f},
    Angle{-0.649649f /* std::numbers::pi_v<float> / 4 */},
    Angle{-std::numbers::pi_v<float> / 2},
};
static_assert(sensor_angles.size() == DistanceSensors::sensor_count);

DistanceSensors &DistanceSensors::get_instance() noexcept
{
    static QWIICMUX mux;
    static SFEVL53L1X current_distance_sensor;
    static constinit DistanceSensors instance(mux, current_distance_sensor);
    return instance;
}

void DistanceSensors::init(TwoWire &i2c, Vl53l1cdTimingBudget timing_budget) noexcept
{
    if (!m_mux.begin(QWIIC_MUX_DEFAULT_ADDRESS, i2c))
    {
        led::infinite_loop([] { ESP_LOGE("sensor", "Mux failed to begin. Please check wiring. Freezing..."); });
    }

    for (auto &&sensor : m_sensors)
    {
        m_mux.setPort(sensor.port);
        if (m_current_distance_sensor.begin(i2c) != 0)  // Begin returns 0 on a good init
        {
            led::infinite_loop(
                [&]
                {
                    ESP_LOGE(
                        "sensor",
                        "Sensor at port %d failed to begin. Please check wiring. Freezing...",
                        sensor.port
                    );
                }
            );
        }

        m_current_distance_sensor.setDistanceModeShort();
        m_current_distance_sensor.setTimingBudgetInMs(timing_budget);
        // Intermeasurement period must be >= timing budget. Default = 100 ms.
        m_current_distance_sensor.setIntermeasurementPeriod(timing_budget);
        m_current_distance_sensor.startRanging();  // Start only once (and never call stop)

        ESP_LOGI("sensor", "Sensor at port %d is online!", sensor.port);
    }
}

std::array<meters, DistanceSensors::sensor_count> DistanceSensors::read_all()
{
    static constinit std::array<meters, DistanceSensors::sensor_count> measurements;
    for (std::size_t i = 0; i < sensor_count; i++)
    {
        m_mux.setPort(m_sensors[i].port);
        while (!m_current_distance_sensor.checkForDataReady())
        {
            delay(1);
        }

        // Get the result of the measurement from the sensor:
        m_sensors[i].distance = m_current_distance_sensor.getDistance();
        m_current_distance_sensor.clearInterrupt();
        measurements[i] = m_sensors[i].get_distance();
    }

    return measurements;
}

static auto predict_distance(const Position &pos, std::size_t sensor_index, const std::span<const Segment> &maze_map)
    noexcept
{
    const auto ray_x = pos.x.get() + sensor_disposition[sensor_index].first.get() * std::cos(pos.theta)
                     - sensor_disposition[sensor_index].second.get() * std::sin(pos.theta);
    const auto ray_y = pos.y.get() + sensor_disposition[sensor_index].first.get() * std::sin(pos.theta)
                     + sensor_disposition[sensor_index].second.get() * std::cos(pos.theta);
    const auto sensor_angle = pos.theta + sensor_angles[sensor_index];
    const Segment sensor_ray{
        Eigen::Vector2f{ray_x.count(), ray_y.count()},
        Eigen::Vector2f{
            (ray_x + max_predict_range * std::cos(sensor_angle)).count(),
            (ray_y + max_predict_range * std::sin(sensor_angle)).count(),
        },
    };

    auto make_result = [&](const auto &wall)
    { return std::pair<meters, const Segment &>(sensor_ray.intersection_distance(wall), wall); };

    auto all_distances = maze_map | std::views::transform(make_result);
    return *std::ranges::min_element(all_distances, [](const auto &a, const auto &b) { return a.first < b.first; });
}

std::pair<DistanceSensors::Measurements, DistanceSensors::Jacobian> DistanceSensors::predict(
    const Position &pos,
    const std::span<const Segment> &maze_map
) noexcept
{
    Measurements error = Measurements::Zero();
    Jacobian jacobian = Jacobian::Zero();
    for (std::size_t i = 0; i < sensor_count; i++)
    {
        const auto measured = m_sensors[i].get_distance();
        if (measured > max_sensor_range)
        {
            continue;
        }

        const auto [distance, wall] = predict_distance(pos, i, maze_map);
        if (distance > max_predict_range)
        {
            continue;
        }

        const auto wall_coefficients = wall.line().coeffs();
        const auto A = wall_coefficients.x();
        const auto B = wall_coefficients.y();
        const auto C = wall_coefficients.z();
        const auto theta = pos.theta + sensor_angles[i];
        const auto denominator = A * std::cos(theta) + B * std::sin(theta);

        error(i) = (measured - distance).count();
        jacobian(i, 0) = -A / denominator;  // Partial derivative w.r.t. x
        jacobian(i, 1) = -B / denominator;  // Partial derivative w.r.t. y
        jacobian(i, 2) = (A * pos.x->count() + B * pos.y->count() + C) * (B * std::cos(theta) - A * std::sin(theta))
                       / std::pow(denominator, 2);  // Partial derivative w.r.t. theta
    }

    return std::pair(error, jacobian);
}
