#include <misc_utils/physical_size.h>

#include "algorithm_api_mock.h"
#include "debug_utils.h"
#include "distance_sensor.h"
#include "kalman_filter.h"
#include "motion_model.h"
#include "motor.h"
#include "periodic_caller.h"
#include "pid_controller.h"
#include "temp_map.h"
#include <sdkconfig.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numbers>
#include <ratio>

#include <Arduino.h>
#include <SparkFun_I2C_Mux_Arduino_Library.h>
#include <SparkFun_VL53L1X.h>
#include <Wire.h>

using namespace micromouse;
using namespace std::chrono_literals;

static constexpr std::chrono::duration<float, std::micro> pid_loop_period{CONFIG_PID_LOOP_PERIOD};

using MotorSpeed = ConstrainedValue<ValueRange<float, -Motor::max_speed, Motor::max_speed, Mode::Closed>, false>;
using PhysicalMotorSpeed = PhysicalSize<MotorSpeed, meters_per_second::units>;

template <PhysicalSizeType T>
static T abs(const T &val) noexcept
{
    return T{std::abs(val.count())};
}

static constexpr auto is_vertical(Direction d) noexcept
{
    return d == Direction::North || d == Direction::South;
}

/**
 * @brief Finds the closest direction to the given angle by dividing the plane into quarters.
 *
 * @param angle angle to check.
 * @return Closest direction.
 */
static auto to_closest_direction(const Angle &angle) noexcept
{
    if (std::abs(angle) < std::numbers::pi_v<float> / 4)
    {
        return Direction::East;
    }
    else if (std::abs(angle) < 3 * std::numbers::pi_v<float> / 4)
    {
        return (angle.get() > 0) ? Direction::South : Direction::North;
    }

    return Direction::West;
}

struct MotorArgs
{
    Motor motor;
    PidController linear_velocity_distance_pid;
    PidController angular_velocity_angle_pid;
    PidController velocity_pid;
    float distance_linear_Kv;
    float distance_angular_Kv;
    float velocity_Kv;
    float Ks;
    float output;
    PhysicalMotorSpeed wanted_velocity;
    meters_per_second current_velocity;
};

struct PidArgs
{
    MotorArgs left;
    MotorArgs right;
    Position target_pos;
    Position pos;
    bool sensors_available;
};

static KalmanFilter kalman_filter;

/**
 * @brief This function calculates the control signal for each motor.
 * Using those two equations:
 * Vw = Kv1 * sqrt(2 * a * |dx|) * sign(dx) + PID(dx)
 * Vm = Ks + Kv2 * Vw + PID(Vw - Vc)
 * Where:
 * Vw - Wanted velocity.
 * Kv1 - Wanted velocity factor.
 * a - Maximum motor acceleration.
 * dx - Distance error (current pos - wanted pos).
 * Vm - Motor velocity output.
 * Ks - Static motor velocity (the smallest velocity to drive the motor on ground).
 * Kv2 - Motor velocity factor.
 * Vc - Current motor velocity.
 *
 * The first equation uses a PID on the distance error we want to drive and adds a feed
 * forward term which assumes constant acceleration motion (trapezoid motion profile).
 * The second equation is to close the loop on the velocity value sent to the motor.
 * It uses a PID on the velocity error and adds a feed forward term of the wanted velocity.
 *
 * Using the first equation, we calculate the linear and angular velocities, Vl and Va.
 * Vl is calculated from the distance error while Va is calculated from the angle error.
 * The angle error is composed of the error between current and wanted angle and the error between current angle and the
 * driving direction needed to reach the destination.
 * The left motor's wanted speed is Vl + Va and the right motor's wanted speed is Vl - Va.
 * Now we can use the second equation to calculate the velocity of each motor.
 *
 * The left and right motors current velocities change the robot's position using the robot's differential drive motion
 * model.
 * When new data from the distance sensors arrive, we use a Kalman filter to fuse the data from the sensors and the
 * predicted position from the motion model to get a new better estimated position.
 *
 * @param args A pointer to a `PidArgs` struct.
 */
static void pid_loop(void *args) noexcept
{
    static auto &distance_sensors = DistanceSensors::get_instance();

    // Initialization:
    using LinearMotorSpeed = ConstrainedValue<
        ValueRange<float, MotorSpeed::range_type::low / 12.0f, MotorSpeed::range_type::high / 12.0f, Mode::Closed>,
        false>;
    using RotationalMotorSpeed = ConstrainedValue<
        ValueRange<float, MotorSpeed::range_type::low / 16.0f, MotorSpeed::range_type::high / 16.0f, Mode::Closed>,
        false>;

    static_assert(
        MotorSpeed::range_type::contains(LinearMotorSpeed::range_type::low)
        && MotorSpeed::range_type::contains(LinearMotorSpeed::range_type::high)
    );
    static_assert(
        MotorSpeed::range_type::contains(RotationalMotorSpeed::range_type::low)
        && MotorSpeed::range_type::contains(RotationalMotorSpeed::range_type::high)
    );
    static_assert(
        MotorSpeed::range_type::contains(LinearMotorSpeed::range_type::low + RotationalMotorSpeed::range_type::low)
        && MotorSpeed::range_type::contains(LinearMotorSpeed::range_type::high + RotationalMotorSpeed::range_type::high)
    );

    // Update pos:
    PidArgs *pid_args = static_cast<PidArgs *>(args);
    pid_args->left.current_velocity = pid_args->left.motor.get_speed(pid_loop_period);
    pid_args->right.current_velocity = pid_args->right.motor.get_speed(pid_loop_period);
    const auto predicted_pos =
        update_pos(pid_args->pos, pid_args->left.current_velocity, pid_args->right.current_velocity, pid_loop_period);
    if (pid_args->sensors_available)
    {
        pid_args->sensors_available = false;
        const auto pos_j = pos_jacobian(
            pid_args->pos,
            pid_args->left.current_velocity,
            pid_args->right.current_velocity,
            pid_loop_period
        );
        const auto [distance_sensor_error, distance_sensor_jacobian] =
            distance_sensors.predict(pid_args->pos, maze_map);
        // Maybe compensate for the sensors coming later so the reading was in the past...
        pid_args->pos = kalman_filter(predicted_pos, pos_j, distance_sensor_error, distance_sensor_jacobian);
    }
    else
    {
        pid_args->pos = predicted_pos;
    }

    // Calculate error
    const auto x_err = unit_cast<millimeters>((pid_args->target_pos.x - pid_args->pos.x).get());
    const auto y_err = unit_cast<millimeters>((pid_args->target_pos.y - pid_args->pos.y).get());
    const auto dist_sign = std::copysign(
        1.0f,
        x_err.count() * std::cos(pid_args->pos.theta) + y_err.count() * std::sin(pid_args->pos.theta)
    );
    const auto dist_err = dist_sign * std::sqrt(std::pow(x_err.count(), 2.0f) + std::pow(y_err.count(), 2.0f));
    const auto direction_err = [&]()
    {
        if (dist_err > (50_mm).count())
        {
            return static_cast<float>(Angle{std::atan2(y_err.count(), x_err.count())} - pid_args->pos.theta);
        }
        else
        {
            return 0.0f;  // Ignore direction error when close to the target
        }
    }();
    const auto angle_err = static_cast<float>(pid_args->target_pos.theta - pid_args->pos.theta) + direction_err;

    // Calculate wanted velocity from distance and angle errors:
    const auto calc_wanted_velocity = [&](PidController &pid, float err, float kv)
    {
        // Use PID on the error and add a feed forward term which assumes constant acceleration motion (trapezoid motion
        // profile).
        return meters_per_second{
            std::copysign(kv, err) * std::sqrt(2 * Motor::max_acceleration * std::abs(err)) + pid.calculate_pid(err)
        };
    };
    const auto limit_velocity = [&](float new_velocity, float last_velocity)
    {
        static constexpr auto max_accelaration =
            Motor::max_acceleration * duration_cast<std::chrono::duration<float>>(pid_loop_period).count();
        return PhysicalMotorSpeed{
            std::clamp(new_velocity, last_velocity - max_accelaration, last_velocity + max_accelaration)
        };
    };
    const auto calc_motor_velocity = [&](MotorArgs &motor, bool left, float dist_err, float angle_err)
    {
        const LinearMotorSpeed wanted_linear_velocity{
            calc_wanted_velocity(motor.linear_velocity_distance_pid, dist_err, motor.distance_linear_Kv).count()
        };
        const RotationalMotorSpeed wanted_angular_velocity{
            calc_wanted_velocity(motor.angular_velocity_angle_pid, angle_err, motor.distance_angular_Kv).count()
        };

        motor.wanted_velocity = limit_velocity(
            left ? wanted_linear_velocity.get() + wanted_angular_velocity.get()
                 : wanted_linear_velocity.get() - wanted_angular_velocity.get(),
            motor.wanted_velocity.count().get()
        );
        return std::copysign(motor.Ks, motor.wanted_velocity.count())
             + motor.velocity_Kv * motor.wanted_velocity.count()
             + motor.velocity_pid.calculate_pid(motor.wanted_velocity.count() - motor.current_velocity.count());
    };
    const MotorSpeed left_motor_velocity{calc_motor_velocity(pid_args->left, true, dist_err, angle_err)};
    const MotorSpeed right_motor_velocity{calc_motor_velocity(pid_args->right, false, dist_err, angle_err)};

    // Outputs:
    pid_args->left.output = Motor::bdc_mcpwm_duty_tick_max * left_motor_velocity / Motor::max_speed;
    pid_args->right.output = Motor::bdc_mcpwm_duty_tick_max * right_motor_velocity / Motor::max_speed;
    pid_args->left.motor.set_pwm(pid_args->left.output);
    pid_args->right.motor.set_pwm(pid_args->right.output);
}

static void print_log(const PidArgs &args, const std::chrono::milliseconds &cycle_time = 0ms) noexcept
{
    std::printf(
        "WPos: [ %g  %g  %g ] Pos: [ %g  %g  %g ] L: Actual = %g Wanted = %g Output = %g R: Actual = %g Wanted = %g "
        "Output = %g Cycle time: %lld\n",
        args.target_pos.x->count(),
        args.target_pos.y->count(),
        args.target_pos.theta.get(),
        args.pos.x->count(),
        args.pos.y->count(),
        args.pos.theta.get(),
        args.left.current_velocity.count(),
        args.left.wanted_velocity.count().get(),
        args.left.output,
        args.right.current_velocity.count(),
        args.right.wanted_velocity.count().get(),
        args.right.output,
        cycle_time.count()
    );
}

static auto now() noexcept
{
    return std::chrono::milliseconds{millis()};
}

// WARNING: if program reaches end of function app_main() the MCU will restart.
extern "C" void app_main()
{
    initArduino();

    static const auto led_pin = GPIO_NUM_13;
    pinMode(led_pin, OUTPUT);
    auto led_state = false;

    constexpr int sda_pin = GPIO_NUM_23;
    constexpr int scl_pin = GPIO_NUM_22;
    TwoWire i2c(0);
    i2c.setPins(sda_pin, scl_pin);
    i2c.begin();

    auto &distance_sensors = DistanceSensors::get_instance();
    distance_sensors.init(i2c);

    static constexpr float d_kv = 0.014f;
    static constexpr float d_kp = 0.0005f;
    static constexpr float d_ki = 0.0f;
    static constexpr float d_kd = 0.005f;

    static constexpr float a_kv = 0.1f;
    static constexpr float a_kp = 0.05f;
    static constexpr float a_ki = 0.0f;
    static constexpr float a_kd = 0.005f;

    static constexpr float v_kv = 1.0f;
    static constexpr float v_kp = 3.0f;
    static constexpr float v_ki = 0.0f;
    static constexpr float v_kd = 0.25f;

    AlgorithmApi algorithm;
    const auto start_pos = *algorithm.get_next();
    auto alg_pos = algorithm.get_next();
    PidArgs pid_args{
        .left{
            .motor{GPIO_NUM_15, GPIO_NUM_32, GPIO_NUM_14, GPIO_NUM_21, LeftMotor, true},
            .linear_velocity_distance_pid{d_kp, d_ki, d_kd},
            .angular_velocity_angle_pid{a_kp, a_ki, a_kd},
            .velocity_pid{v_kp, v_ki, v_kd},
            .distance_linear_Kv = d_kv,
            .distance_angular_Kv = a_kv,
            .velocity_Kv = v_kv,
            .Ks = 0.71f,
            .output = 0.0f,
            .wanted_velocity{},
            .current_velocity{},
        },
        .right{
            .motor{GPIO_NUM_33, GPIO_NUM_27, GPIO_NUM_12, GPIO_NUM_18, RightMotor, true},
            .linear_velocity_distance_pid{d_kp, d_ki, d_kd},
            .angular_velocity_angle_pid{a_kp, a_ki, a_kd},
            .velocity_pid{v_kp, v_ki, v_kd},
            .distance_linear_Kv = d_kv,
            .distance_angular_Kv = a_kv,
            .velocity_Kv = v_kv,
            .Ks = 0.71f,
            .output = 0.0f,
            .wanted_velocity{},
            .current_velocity{},
        },
        .target_pos{start_pos},
        .pos{start_pos},
        .sensors_available = false,
    };

    // start delay, log setup and sensors warmup:
    print_log(pid_args);
    sleep(10);
    for (auto i = 0; i < DistanceSensors::avg_filter_size; i++)  // warmup
    {
        distance_sensors.read_all();
    }
    print_log(pid_args);

    static constexpr auto max_diff_distance = 20.0_mm;
    static constexpr Angle max_diff_angle{std::numbers::pi_v<float> / 60};

    // start PID task
    pid_args.left.motor.clear_encoder();
    pid_args.right.motor.clear_encoder();
    PeriodicCaller pid_caller(pid_loop, static_cast<void *>(&pid_args));
    pid_caller.start(pid_loop_period);

    static constexpr auto loop_interval = 20ms;
    static_assert(loop_interval >= 20ms, "loop_interval doesn't match with sensor reading times");
    // Main loop
    while (true)
    {
        const auto cycle_start_time = now();
        distance_sensors.read_all();
        pid_args.sensors_available = true;
        if (alg_pos)
        {
            const auto &next_pos = *alg_pos;
            pid_args.target_pos = next_pos;
            const auto x_err = unit_cast<millimeters>((next_pos.x - pid_args.pos.x).get());
            const auto y_err = unit_cast<millimeters>((next_pos.y - pid_args.pos.y).get());
            const auto pos_angle_err = next_pos.theta - pid_args.pos.theta;
            const auto next_direction = to_closest_direction(next_pos.theta);
            if (std::abs(pos_angle_err.get()) <= max_diff_angle.get()
                && abs(is_vertical(next_direction) ? y_err : x_err) <= max_diff_distance)
            {
                pid_args.pos = next_pos;  // snap
                alg_pos = algorithm.get_next();
                led_state = !led_state;
                digitalWrite(led_pin, led_state);
            }
        }
        else
        {
            pid_caller.stop();
            pid_args.left.motor.set_pwm(0.0f);
            pid_args.right.motor.set_pwm(0.0f);
        }

        debug_utils::halt_if_input(
            debug_utils::OnHalt(
                [&]
                {
                    pid_caller.stop();
                    pid_args.left.motor.set_pwm(0.0f);
                    pid_args.right.motor.set_pwm(0.0f);
                }
            ),
            debug_utils::OnResume([&] { pid_caller.start(pid_loop_period); }),
            debug_utils::Verbosity::Silent
        );

        // keep loop time at least loop_interval.
        while (now() - cycle_start_time < loop_interval)
        {
            delay(1);
        }

        print_log(pid_args, now() - cycle_start_time);
    }
}
