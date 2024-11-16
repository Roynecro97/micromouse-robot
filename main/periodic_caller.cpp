#include "periodic_caller.h"

periodic_caller::periodic_caller(esp_timer_cb_t callback, void* args)
    : m_loop_timer{nullptr}
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = callback,
        .arg = args,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "periodic",
        .skip_unhandled_events = false,
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &m_loop_timer));
}

periodic_caller::~periodic_caller()
{
    if (esp_timer_is_active(m_loop_timer))
    {
        ESP_ERROR_CHECK(esp_timer_stop(m_loop_timer));
    }

    ESP_ERROR_CHECK(esp_timer_delete(m_loop_timer));
}

void periodic_caller::start(uint32_t period)
{
    if (esp_timer_is_active(m_loop_timer))
    {
        ESP_ERROR_CHECK(esp_timer_stop(m_loop_timer));
    }

    ESP_ERROR_CHECK(esp_timer_start_periodic(m_loop_timer, period));
}

void periodic_caller::stop()
{
    if (esp_timer_is_active(m_loop_timer))
    {
        ESP_ERROR_CHECK(esp_timer_stop(m_loop_timer));
    }
}
