#include "motor.h"

#include <esp_timer.h>
#include <numbers>

Motor::Motor(gpio_num_t mcpwm_A, gpio_num_t mcpwm_B, gpio_num_t enc_A, gpio_num_t enc_B, motor_ID id, bool reversed)
    : m_motor{nullptr}
    , m_pcnt_encoder{nullptr}
    , m_last_pulse_count{0}
    , m_id{id}
    , m_reversed{reversed}
{
    // setup motor:
    bdc_motor_config_t motor_config = {
        .pwma_gpio_num = static_cast<uint32_t>(mcpwm_A),
        .pwmb_gpio_num = static_cast<uint32_t>(mcpwm_B),
        .pwm_freq_hz = bdc_mcpwm_freq_hz,
    };

    bdc_motor_mcpwm_config_t mcpwm_config = {
        .group_id = m_id,
        .resolution_hz = bdc_mcpwm_timer_resolution_hz,
    };

    ESP_ERROR_CHECK(bdc_motor_new_mcpwm_device(&motor_config, &mcpwm_config, &m_motor));
    ESP_ERROR_CHECK(bdc_motor_enable(m_motor));

    // setup encoder:
    pcnt_unit_config_t unit_config = {
        .low_limit = bdc_encoder_pcnt_low_limit,
        .high_limit = bdc_encoder_pcnt_high_limit,
        .intr_priority = 0,
        .flags = {.accum_count = true}, // enable counter accumulation
    };

    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &m_pcnt_encoder));
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };

    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(m_pcnt_encoder, &filter_config));
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = enc_A,
        .level_gpio_num = enc_B,
        .flags = {},
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(m_pcnt_encoder, &chan_a_config, &pcnt_chan_a));

    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = enc_B,
        .level_gpio_num = enc_A,
        .flags = {},
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(m_pcnt_encoder, &chan_b_config, &pcnt_chan_b));

    if (m_reversed)
    {
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    }
    else
    {
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    }

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(m_pcnt_encoder, bdc_encoder_pcnt_high_limit));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(m_pcnt_encoder, bdc_encoder_pcnt_low_limit));
    ESP_ERROR_CHECK(pcnt_unit_enable(m_pcnt_encoder));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(m_pcnt_encoder));
    ESP_ERROR_CHECK(pcnt_unit_start(m_pcnt_encoder));
}

Motor::~Motor()
{
    ESP_ERROR_CHECK(bdc_motor_disable(m_motor));
    ESP_ERROR_CHECK(bdc_motor_del(m_motor));

    ESP_ERROR_CHECK(pcnt_unit_stop(m_pcnt_encoder));
    ESP_ERROR_CHECK(pcnt_unit_disable(m_pcnt_encoder));
    ESP_ERROR_CHECK(pcnt_del_unit(m_pcnt_encoder));
}

int Motor::get_enc_ticks() const
{
    int tick_count = 0;
    pcnt_unit_get_count(m_pcnt_encoder, &tick_count);
    return tick_count;
}

float Motor::ticks_to_distance(int ticks)
{
    static constexpr int ticks_per_rotation = 336; // Measured by a single turn of the wheel
    static constexpr float perimeter = std::numbers::pi_v<float> * 0.032f; // The wheels' diameter is 3.2 [cm]
    return (static_cast<float>(ticks * perimeter) / ticks_per_rotation);
}

float Motor::ticks_to_velocity(int ticks, unsigned int time) const
{
    return (ticks_to_distance(ticks) * 1000 / time);
}

float Motor::get_speed(unsigned int time_frame)
{
    const int cur_pulse_count = get_enc_ticks();
    float speed = ticks_to_velocity(cur_pulse_count - m_last_pulse_count, time_frame);
    m_last_pulse_count = cur_pulse_count;
    return speed;
}

void Motor::set_pwm(float duty_cycle)
{
    if (duty_cycle < 0.0f)
    {
        duty_cycle = -duty_cycle;
        if (m_reversed)
        {
            bdc_motor_forward(m_motor);
        }
        else
        {
            bdc_motor_reverse(m_motor);
        }
    }
    else if (m_reversed)
    {
        bdc_motor_reverse(m_motor);
    }
    else
    {
        bdc_motor_forward(m_motor);
    }

    bdc_motor_set_speed(m_motor, static_cast<uint32_t>(duty_cycle));
}
