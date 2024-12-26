#ifndef MAIN_PID_CONTROLLER_H
#define MAIN_PID_CONTROLLER_H

#include "motor.h"

#include <pid_ctrl.h>

class PidController
{
public:
    static constexpr int max_pid_integral_value = Motor::bdc_mcpwm_duty_tick_max / 2;

    PidController(float kp, float ki = 0, float kd = 0) noexcept;
    ~PidController();
    PidController(const PidController &) noexcept = delete;
    PidController(PidController &&) noexcept = delete;
    PidController &operator=(const PidController &) noexcept = delete;
    PidController &operator=(PidController &&) noexcept = delete;

    /**
     * @brief Use wanted value and actual value with PID constants to produce the control value.
     *
     * @param error Difference between wanted value and actual value.
     * @return The PID calculated value.
     */
    float calculate_pid(float error) noexcept;

    inline float get_pid_val() const noexcept { return m_pid_val; };
    inline pid_ctrl_block_handle_t get_pid() const noexcept { return m_pid_ctrl; };

private:
    float m_pid_val;
    pid_ctrl_block_handle_t m_pid_ctrl;
};

#endif  // MAIN_PID_CONTROLLER_H
