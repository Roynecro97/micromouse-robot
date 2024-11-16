#include "algorithm_api_mock.h"
#include "average_filter.h"
#include "motor.h"
#include "periodic_caller.h"
#include "pid_controller.h"

#include <Arduino.h>
#include <SparkFun_I2C_Mux_Arduino_Library.h>
#include <SparkFun_VL53L1X.h>
#include <bdc_motor.h>
#include <pid_ctrl.h>

#include <array>

static constexpr unsigned int pid_loop_period_us = 5 * 1'000;        // calculate PID every 10ms
static constexpr unsigned int VL53L1CD_loop_period_us = 20 * 1'000;  // calculate distance every 20ms

enum VL53L1CD_TimingBudget : uint16_t
{
    VL53L1CD_TimingBudget_15ms = 15,
    VL53L1CD_TimingBudget_20ms = 20,
    VL53L1CD_TimingBudget_33ms = 33,
    VL53L1CD_TimingBudget_50ms = 50,
    VL53L1CD_TimingBudget_100ms = 100,
    VL53L1CD_TimingBudget_200ms = 200,
    VL53L1CD_TimingBudget_500ms = 500,
};

struct VL53L1CD_args
{
    QWIICMUX &mux;
    SFEVL53L1X &distance_sensor;
    std::array<uint8_t, 5> &sensor_port;
    std::array<avg_filter<uint16_t, 10, int>, 5> &sensor_distance;
};

static void VL53L1CD_loop(void *args)
{
    VL53L1CD_args *vl53l1cd_args = static_cast<VL53L1CD_args *>(args);
    for (uint32_t i = 0; i < vl53l1cd_args->sensor_port.size(); i++)
    {
        vl53l1cd_args->mux.setPort(vl53l1cd_args->sensor_port[i]);
        while (!vl53l1cd_args->distance_sensor.checkForDataReady())
        {
            delay(1);
        }

        // Get the result of the measurement from the sensor:
        vl53l1cd_args->sensor_distance[i] = vl53l1cd_args->distance_sensor.getDistance();
        vl53l1cd_args->distance_sensor.clearInterrupt();
    }
}

struct PID_args
{
    Motor &motor;
    PID_controller &distance_pid;
    PID_controller &velocity_pid;
    float delta_distance;
    float motor_output;
    float Vw;
    float Vc;
    float distance_Kv;
    float velocity_Kv;
    float Ks;
};

/**
 * @brief This function calculates the control signal for each motor.
 * Using those two equations:
 * Vw = Kv1 * sqrt(2 * a * |Δx|) * sign(Δx) + PID(Δx)
 * Vm = Ks + Kv2 * Vw + PID(Vw - Vc)
 * Where:
 * Vw - Wanted velocity.
 * Kv1 - Wanted velocity factor (constant).
 * a - Robot's acceleration.
 * Δx - Distance left to drive (current pos minus wanted pos).
 * Vm - Motor velocity (output velocity to motor).
 * Ks - Static motor velocity (the smallest velocity to drive the motor on ground). Can be different for each motor.
 * Kv2 - Motor velocity factor (constant).
 * Vc - Current motor velocity.
 *
 * The first equation uses a PID on the distance error we want to drive and adds a feed
 * forward term which assumes constant acceleration motion (trapezoid motion profile).
 * The second equation is to close the loop on the velocity value sent to the motor.
 * It uses a PID on the velocity error and adds a feed forward term of the wanted velocity.
 *
 * @param args A pointer to a `PID_args` struct.
 */
static void pid_loop(void *args)
{
    static constexpr unsigned int pid_loop_period_ms = pid_loop_period_us / 1'000;
    PID_args *pid_args = static_cast<PID_args *>(args);
    const float delta_distance = pid_args->delta_distance;
    const float Kv1 = pid_args->distance_Kv;
    const float wanted_velocity = std::max(
        std::min(
            Kv1 * std::sqrt(2 * Motor::max_motor_acceleration * std::abs(delta_distance))
                    * std::copysignf(1.0f, delta_distance)
                + pid_args->distance_pid.calculate_pid(delta_distance),
            Motor::max_motor_speed
        ),
        -Motor::max_motor_speed
    );
    Motor &motor = pid_args->motor;
    const float current_velocity = motor.get_speed(pid_loop_period_ms);
    const float Kv2 = pid_args->velocity_Kv;
    const float motor_velocity = std::max(
        std::min(
            pid_args->Ks * std::copysignf(1.0f, delta_distance) + Kv2 * wanted_velocity
                + pid_args->velocity_pid.calculate_pid(wanted_velocity - current_velocity),
            Motor::max_motor_speed
        ),
        -Motor::max_motor_speed
    );
    pid_args->motor_output = Motor::bdc_mcpwm_duty_tick_max * motor_velocity / Motor::max_motor_speed;
    pid_args->Vw = wanted_velocity;
    pid_args->Vc = current_velocity;
    motor.set_pwm(pid_args->motor_output);
}

// WARNING: if program reaches end of function app_main() the MCU will restart.
extern "C" void app_main()
{
    initArduino();
    // Serial.begin(115200);
    // while(!Serial); // wait for serial port to connect

    constexpr int sda_pin = GPIO_NUM_23;
    constexpr int scl_pin = GPIO_NUM_22;
    TwoWire i2c(0);
    i2c.setPins(sda_pin, scl_pin);
    i2c.begin();

    QWIICMUX mux;
    if (!mux.begin(QWIIC_MUX_DEFAULT_ADDRESS, i2c))
    {
        printf("Mux failed to begin. Please check wiring. Freezing...\n");
        while (1)
            ;
    }

    SFEVL53L1X distance_sensor;
    std::array<uint8_t, 5> sensor_port{1, 2, 3, 4, 5};
    for (uint32_t i = 0; i < sensor_port.size(); i++)
    {
        mux.setPort(sensor_port[i]);
        if (distance_sensor.begin(i2c) != 0)  // Begin returns 0 on a good init
        {
            printf("Sensor at port %d failed to begin. Please check wiring. Freezing...\n", sensor_port[i]);
            while (1)
                ;
        }

        distance_sensor.setDistanceModeShort();
        distance_sensor.setTimingBudgetInMs(VL53L1CD_TimingBudget_20ms);
        // Intermeasurement period must be >= timing budget. Default = 100 ms.
        distance_sensor.setIntermeasurementPeriod(VL53L1CD_TimingBudget_20ms);
        distance_sensor.startRanging();  // Start only once (and never call stop)

        printf("Sensor at port %d is online!\n", sensor_port[i]);
    }

    Motor left_motor(GPIO_NUM_15, GPIO_NUM_32, GPIO_NUM_14, GPIO_NUM_21, LeftMotor, true);
    Motor right_motor(GPIO_NUM_33, GPIO_NUM_27, GPIO_NUM_12, GPIO_NUM_13, RightMotor, true);
    constexpr float d_kv = 0.014f;
    constexpr float d_kp = 0.0005f;
    constexpr float d_ki = 0.0f;
    constexpr float d_kd = 0.005f;
    PID_controller left_motor_distance(d_kp, d_ki, d_kd);
    PID_controller right_motor_distance(d_kp, d_ki, d_kd);
    constexpr float v_kp = 8.2f;
    constexpr float v_ki = 0.0f;
    constexpr float v_kd = 0.5f;
    PID_controller left_motor_velocity(v_kp, v_ki, v_kd);
    PID_controller right_motor_velocity(v_kp, v_ki, v_kd);
    PID_args left_args = {
        .motor = left_motor,
        .distance_pid = left_motor_distance,
        .velocity_pid = left_motor_velocity,
        .delta_distance = 0.0f,
        .motor_output = 0.0f,
        .Vw = 0.0f,
        .Vc = 0.0f,
        .distance_Kv = d_kv,
        .velocity_Kv = 1.5f,
        .Ks = 0.0f,
    };
    PID_args right_args = {
        .motor = right_motor,
        .distance_pid = right_motor_distance,
        .velocity_pid = right_motor_velocity,
        .delta_distance = 0.0f,
        .motor_output = 0.0f,
        .Vw = 0.0f,
        .Vc = 0.0f,
        .distance_Kv = d_kv,
        .velocity_Kv = 1.5f,
        .Ks = 0.0f,
    };
    periodic_caller left_pid_caller(pid_loop, static_cast<void *>(&left_args));
    periodic_caller right_pid_caller(pid_loop, static_cast<void *>(&right_args));
    left_pid_caller.start(pid_loop_period_us);
    right_pid_caller.start(pid_loop_period_us);

    constexpr uint16_t wanted_drive_distance_mm = 300;
    constexpr uint16_t wanted_wall_distance_mm = 36;  // Robot is 108 [mm] wide.
    constexpr uint16_t front_wall_distance_for_turn_mm = 50;

    // log setup:
    printf("L Distance (mm) = %g ", 0.0f);
    printf("R Distance (mm) = %g ", 0.0f);
    printf("L: Actual = %g Wanted = %g Output = %g ", 0.0f, 0.0f, 0.0f);
    printf("R: Actual = %g Wanted = %g Output = %g ", 0.0f, 0.0f, 0.0f);
    printf("Cycle time (ms): %d\n", 0);
    sleep(10);
    printf("L Distance (mm) = %g ", 0.0f);
    printf("R Distance (mm) = %g ", 0.0f);
    printf("L: Actual = %g Wanted = %g Output = %g ", 0.0f, 0.0f, 0.0f);
    printf("R: Actual = %g Wanted = %g Output = %g ", 0.0f, 0.0f, 0.0f);
    printf("Cycle time (ms): %d\n", 0);

    int left_start_ticks = left_motor.get_enc_ticks();
    int right_start_ticks = right_motor.get_enc_ticks();

    std::array<avg_filter<uint16_t, 10, int>, 5> sensor_distance;
    /*
    VL53L1CD_args distance_sensor_args = {
        .mux = mux,
        .distance_sensor = distance_sensor,
        .sensor_port = sensor_port,
        .sensor_distance = sensor_distance,
    };

    periodic_caller distance_sensor_caller(VL53L1CD_loop, static_cast<void*>(&distance_sensor_args));
    distance_sensor_caller.start(VL53L1CD_loop_period_us);
    */

    bool first_finish_cycle = false;
    uint32_t finish_cycles = 0;
    constexpr uint32_t finish_cycles_needed = 5;
    constexpr float goal_distance = 2.0f;
    algorithm_api algorithm;
    cell_coordinate current_cell{0, 0};
    heading current_heading = heading::north;
    // std::array<motor_distance, 8> distances{
    //     motor_distance{wanted_drive_distance_mm, wanted_drive_distance_mm},
    //     motor_distance{turn_90_wheel_distance_mm, 0.0f},
    //     motor_distance{wanted_drive_distance_mm, wanted_drive_distance_mm},
    //     motor_distance{turn_90_wheel_distance_mm, 0.0f},
    //     motor_distance{wanted_drive_distance_mm, wanted_drive_distance_mm},
    //     motor_distance{turn_90_wheel_distance_mm, 0.0f},
    //     motor_distance{wanted_drive_distance_mm, wanted_drive_distance_mm},
    //     motor_distance{turn_90_wheel_distance_mm, 0.0f},
    // };

    auto next_cell = algorithm.get_next();

    // Main loop
    while (true)
    {
        unsigned long cycle_time = millis();
        for (uint32_t i = 0; i < sensor_port.size(); i++)
        {
            mux.setPort(sensor_port[i]);
            while (!distance_sensor.checkForDataReady())
            {
                delay(1);
            }

            // Get the result of the measurement from the sensor:
            sensor_distance[i] = distance_sensor.getDistance();
            distance_sensor.clearInterrupt();
        }

        /*
        const float front_distance = sensor_distance[2] - wanted_drive_distance_mm;
        const float left_distance = sensor_distance[4] - wanted_drive_distance_mm;
        left_args.delta_distance = front_distance - left_distance;
        right_args.delta_distance = front_distance + left_distance;
        printf("Distance (mm) port %" PRIu8 " = %d ", sensor_port[2], sensor_distance[2].avg());
        printf("Distance (mm) port %" PRIu8 " = %d ", sensor_port[4], sensor_distance[4].avg());
        printf("L: Actual = %g Wanted = %g Output = %g ",
                left_args.Vc, left_args.Vw, left_args.motor_output);
        printf("R: Actual = %g Wanted = %g Output = %g ",
                right_args.Vc, right_args.Vw, right_args.motor_output);
        cycle_time = millis() - cycle_time;
        */

        const float left_motor_distance_mm =
            1'000 * Motor::ticks_to_distance(left_motor.get_enc_ticks() - left_start_ticks);
        const float right_motor_distance_mm =
            1'000 * Motor::ticks_to_distance(right_motor.get_enc_ticks() - right_start_ticks);
        if (next_cell)
        {
            const auto command = cell_coordinate::to_command(current_cell, current_heading, next_cell.value());
            const auto distance = command.first.to_distance();
            const float left_distance = distance.left_distance - left_motor_distance_mm;
            const float right_distance = distance.right_distance - right_motor_distance_mm;
            // const float wall_distance = wanted_wall_distance_mm - sensor_distance[4];

            left_args.delta_distance = left_distance;
            right_args.delta_distance = right_distance;
            if (left_args.delta_distance < goal_distance && right_args.delta_distance < goal_distance)
            {
                finish_cycles++;
                if (finish_cycles > finish_cycles_needed)
                {
                    if (!first_finish_cycle)
                    {
                        left_start_ticks = left_motor.get_enc_ticks();
                        right_start_ticks = right_motor.get_enc_ticks();
                        printf(
                            "current cell: (%d, %d) next cell: (%d, %d) heading: %d\n",
                            current_cell.x,
                            current_cell.y,
                            next_cell.value().x,
                            next_cell.value().y,
                            static_cast<int>(current_heading)
                        );
                        current_heading = command.second;
                        current_cell = next_cell.value();
                        next_cell = algorithm.get_next();
                    }
                    first_finish_cycle = true;
                }
            }
            else
            {
                finish_cycles = 0;
                first_finish_cycle = false;
            }
        }
        else
        {
            left_args.delta_distance = 0;
            right_args.delta_distance = 0;
        }

        printf("L Distance (mm) = %g ", left_motor_distance_mm);
        printf("R Distance (mm) = %g ", right_motor_distance_mm);
        printf("L: Actual = %g Wanted = %g Output = %g ", left_args.Vc, left_args.Vw, left_args.motor_output);
        printf("R: Actual = %g Wanted = %g Output = %g ", right_args.Vc, right_args.Vw, right_args.motor_output);
        cycle_time = millis() - cycle_time;
        printf("Cycle time (ms): %ld\n", cycle_time);
    }
}
