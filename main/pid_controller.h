#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <pid_ctrl.h>

#include "motor.h"

class PID_controller {
public:
    static constexpr int max_pid_integral_value = Motor::bdc_mcpwm_duty_tick_max / 2;

    PID_controller(float kp, float ki = 0, float kd = 0);
    ~PID_controller();

    /**
     * @brief Use wanted value and actual value with PID constants to produce the control value.
     *
     * @param error Difference between wanted value and actual value.
     * @return The PID calculated value.
     */
    float calculate_pid(float error);

    inline float get_pid_val() const { return m_pid_val; };
    inline pid_ctrl_block_handle_t get_pid() const { return m_pid_ctrl; };

private:
    float m_pid_val;
    pid_ctrl_block_handle_t m_pid_ctrl;
};

#endif
