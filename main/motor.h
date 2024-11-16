#ifndef MOTOR_H
#define MOTOR_H

#include <bdc_motor.h>
#include <driver/pulse_cnt.h>
#include <esp32-hal.h>
#include <pid_ctrl.h>

enum motor_ID : u_int8_t
{
    LeftMotor,
    RightMotor,
};

class Motor {
public:
    static constexpr int bdc_mcpwm_timer_resolution_hz = 10000000; // 10MHz, 1 tick = 0.1us
    static constexpr int bdc_mcpwm_freq_hz = 25000; // 25KHz PWM
    // maximum value we can set for the duty cycle, in ticks
    static constexpr int bdc_mcpwm_duty_tick_max = (bdc_mcpwm_timer_resolution_hz / bdc_mcpwm_freq_hz);
    static constexpr float max_motor_speed = 1.94386f; // [m/s]
    static constexpr float max_motor_acceleration = 9.5f; // [m/s^2]
    static constexpr int bdc_encoder_pcnt_high_limit = 1000;
    static constexpr int bdc_encoder_pcnt_low_limit = -1000;

    Motor(gpio_num_t mcpwm_A, gpio_num_t mcpwm_B, gpio_num_t enc_A, gpio_num_t enc_B, motor_ID id, bool reversed = false);
    ~Motor();
    Motor(const Motor& other) = delete;
    Motor& operator=(const Motor& other) = delete;

    /**
     * @brief Get the encoder tick count.
     *
     * @return Encoder ticks.
     */
    int get_enc_ticks() const;

    /**
     * @brief Converts encoder ticks to wheel distance in [m].
     *
     * @param ticks Encoder ticks.
     * @return Traveled distance.
     */
    static float ticks_to_distance(int ticks);

    /**
     * @brief Converts encoder ticks to wheel speed in [m/s].
     *
     * @param ticks Encoder ticks.
     * @param time Time frame to calculate speed on in [ms].
     * @return Wheel speed.
     */
    float ticks_to_velocity(int ticks, unsigned int time) const;

    /**
     * @brief Calculate the speed of the motor.
     * It is best to call this function every `time_frame`.
     *
     * @param time_frame The time frame to calculate the speed by.
     * The time passed since the last call to this function in [ms].
     * @return Motor speed.
     */
    float get_speed(unsigned int time_frame);

    /**
     * @brief Set the speed for the motor.
     *
     * @param duty_cycle PWM duty cycle. Must be between `-bdc_mcpwm_duty_tick_max` to `bdc_mcpwm_duty_tick_max`.
     */
    void set_pwm(float duty_cycle);

    inline bdc_motor_handle_t get_motor() const { return m_motor; };
    inline pcnt_unit_handle_t get_encoder() const { return m_pcnt_encoder; };
    inline motor_ID get_id() const { return m_id; };

private:
    bdc_motor_handle_t m_motor;
    pcnt_unit_handle_t m_pcnt_encoder;
    int m_last_pulse_count;
    motor_ID m_id;
    bool m_reversed;
};

#endif
