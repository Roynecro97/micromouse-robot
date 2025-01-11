#include "pid_controller.h"

PidController::PidController(float kp, float ki, float kd) noexcept
{
    pid_ctrl_parameter_t pid_runtime_param = {
        .kp = kp,
        .ki = ki,
        .kd = kd,
        .max_output = Motor::bdc_mcpwm_duty_tick_max,
        .min_output = -Motor::bdc_mcpwm_duty_tick_max,
        .max_integral = max_pid_integral_value,
        .min_integral = -max_pid_integral_value,
        .cal_type = PID_CAL_TYPE_POSITIONAL,
    };

    pid_ctrl_config_t pid_config = {
        .init_param = pid_runtime_param,
    };

    ESP_ERROR_CHECK(pid_new_control_block(&pid_config, &m_pid_ctrl));
}

PidController::~PidController()
{
    ESP_ERROR_CHECK(pid_del_control_block(m_pid_ctrl));
}

float PidController::calculate_pid(float error) noexcept
{
    pid_compute(m_pid_ctrl, error, &m_pid_val);
    return m_pid_val;
}
