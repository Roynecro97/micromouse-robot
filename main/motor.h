#ifndef MAIN_MOTOR_H
#define MAIN_MOTOR_H

#include <misc_utils/physical_size.h>

#include <chrono>
#include <limits>

#include <bdc_motor.h>
#include <driver/pulse_cnt.h>
#include <esp32-hal.h>
#include <pid_ctrl.h>

enum MotorID : u_int8_t
{
    LeftMotor,
    RightMotor,
};

class Motor
{
public:
    using Velocity = micromouse::meters_per_second;
    using Distance = micromouse::meters;
    using Time = std::chrono::duration<float, std::milli>;

    static constexpr int bdc_mcpwm_timer_resolution_hz = 10'000'000;  // 10MHz, 1 tick = 0.1us
    static constexpr int bdc_mcpwm_freq_hz = 25'000;                  // 25KHz PWM
    // maximum value we can set for the duty cycle, in ticks
    static constexpr int bdc_mcpwm_duty_tick_max = (bdc_mcpwm_timer_resolution_hz / bdc_mcpwm_freq_hz);
    static constexpr int bdc_encoder_pcnt_high_limit = 1'000;
    static constexpr int bdc_encoder_pcnt_low_limit = -1'000;
    // Theoretical max speed = (max_rpm / 60) * (wheel_gear_teeth_N / motor_gear_teeth_N) * wheel_Perimeter
    static constexpr float max_theoretical_speed = 590.0f / 60.0f * 25.0f / 18.0f * 0.032f * std::numbers::pi_v<float>;
    static constexpr float max_speed = 1.94386f;                            // [m/s]
    static constexpr float max_angular_velocity = 2 * max_speed / 9.9e-2f;  // [rad/s]
    static constexpr float max_acceleration = 9.5f;                         // [m/s^2]

    Motor(gpio_num_t mcpwm_A, gpio_num_t mcpwm_B, gpio_num_t enc_A, gpio_num_t enc_B, MotorID id, bool reversed = false)
        noexcept;
    ~Motor();
    Motor(const Motor &) noexcept = delete;
    Motor(Motor &&) noexcept = delete;
    Motor &operator=(const Motor &) noexcept = delete;
    Motor &operator=(Motor &&) noexcept = delete;

    /**
     * @brief Get the encoder tick count.
     *
     * @return Encoder ticks.
     */
    int get_enc_ticks() const noexcept;

    /**
     * @brief Converts encoder ticks to wheel distance.
     *
     * @param ticks Encoder ticks.
     * @return Traveled distance.
     */
    static Distance ticks_to_distance(int ticks) noexcept;

    /**
     * @brief Converts encoder ticks to wheel speed.
     *
     * @param ticks Encoder ticks.
     * @param time Time frame to calculate speed on.
     * @return Wheel speed.
     */
    Velocity ticks_to_velocity(int ticks, const Time &time) const noexcept;

    /**
     * @brief Calculate the speed of the motor.
     * It is best to call this function every `time_frame`.
     *
     * @param time_frame The time frame to calculate the speed by.
     * The time passed since the last call to this function.
     * @return Motor speed.
     */
    Velocity get_speed(const Time &time_frame) noexcept;

    template <typename Rep, typename Ratio>
    auto get_speed(const std::chrono::duration<Rep, Ratio> &time_frame) noexcept
    {
        return get_speed(duration_cast<Time>(time_frame));
    }

    /**
     * @brief Set the speed for the motor.
     *
     * @param duty_cycle PWM duty cycle. Must be between `-bdc_mcpwm_duty_tick_max` to `bdc_mcpwm_duty_tick_max`.
     */
    void set_pwm(float duty_cycle) noexcept;

    inline bdc_motor_handle_t get_motor() const noexcept { return m_motor; };
    inline pcnt_unit_handle_t get_encoder() const noexcept { return m_pcnt_encoder; };
    inline MotorID get_id() const noexcept { return m_id; };
    inline void clear_encoder() noexcept { ESP_ERROR_CHECK(pcnt_unit_clear_count(m_pcnt_encoder)); };

private:
    bdc_motor_handle_t m_motor;
    pcnt_unit_handle_t m_pcnt_encoder;
    int m_last_pulse_count;
    MotorID m_id;
    bool m_reversed;
};

#endif  // MAIN_MOTOR_H
