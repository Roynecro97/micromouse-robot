#include "periodic_caller.h"

PeriodicCaller::PeriodicCaller(esp_timer_cb_t callback, void *args) noexcept : m_loop_timer{nullptr}
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

PeriodicCaller::~PeriodicCaller()
{
    if (esp_timer_is_active(m_loop_timer))
    {
        ESP_ERROR_CHECK(esp_timer_stop(m_loop_timer));
    }

    ESP_ERROR_CHECK(esp_timer_delete(m_loop_timer));
}

void PeriodicCaller::start(const std::chrono::microseconds &period) noexcept
{
    if (esp_timer_is_active(m_loop_timer))
    {
        ESP_ERROR_CHECK(esp_timer_stop(m_loop_timer));
    }

    ESP_ERROR_CHECK(esp_timer_start_periodic(m_loop_timer, period.count()));
}

void PeriodicCaller::stop() noexcept
{
    if (esp_timer_is_active(m_loop_timer))
    {
        ESP_ERROR_CHECK(esp_timer_stop(m_loop_timer));
    }
}
