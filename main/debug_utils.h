#ifndef MAIN_DEBUG_UTILS_H
#define MAIN_DEBUG_UTILS_H

#include <sdkconfig.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <inttypes.h>
#include <type_traits>
#include <utility>

#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <unistd.h>

namespace debug_utils
{

enum class Verbosity
{
    Silent,
    Normal,
    Verbose,
};

template <typename F>
concept Action = std::is_invocable_v<F>;

namespace detail
{

using namespace std::chrono_literals;

inline constexpr auto grace_period = 1s;

inline auto now() noexcept
{
    return std::chrono::microseconds{esp_timer_get_time()};
}

inline bool read_char(Verbosity verbose_level = Verbosity::Normal)
{
    errno = 0;
    char c;
    const auto read_ok = read(STDIN_FILENO, &c, 1) == 1;
    if (read_ok)
    {
        if (verbose_level >= Verbosity::Normal)
        {
            ESP_LOGI("input", "Found something to read (got %d)!", +c);
        }
    }
    else if (verbose_level >= Verbosity::Verbose)
    {
        ESP_LOGI("input", "Found nothing to read: %d - %s", errno, std::strerror(errno));
    }
    return read_ok;
}

template <Action F>
struct ActionWrapperBase
{
    F &&action;

    template <typename... Args>
    constexpr decltype(auto) operator()(Args &&...args) noexcept(noexcept(action(std::forward<Args>(args)...)))
    {
        return action(std::forward<Args>(args)...);
    }
};

}  // namespace detail

template <Action F>
struct OnHalt : detail::ActionWrapperBase<F>
{
    OnHalt(F &&action) : detail::ActionWrapperBase<F>{std::move(action)} {}
};

template <Action F>
struct OnResume : detail::ActionWrapperBase<F>
{
    OnResume(F &&action) : detail::ActionWrapperBase<F>{std::move(action)} {}
};

inline void halt() noexcept
{
    ESP_LOGI("input", "Halt! Waiting for input...");
    std::uint64_t ticks = 0;
    while (!detail::read_char(Verbosity::Silent))
    {
        vTaskDelay(1);
        ticks++;
    }
    ESP_LOGI("input", "Done! (resuming after %" PRIu64 " ticks)", ticks);
}

template <Action F1, Action F2>
void halt_if_input(OnHalt<F1> on_halt, OnResume<F2> on_resume, Verbosity verbose_level = Verbosity::Normal) noexcept
{
    if (detail::read_char(verbose_level))
    {
        on_halt();
        auto now = detail::now();
        while (detail::now() - now < detail::grace_period)
        {
            // Empty input buffers to prevent spam-clicking from releasing
            if (!detail::read_char(verbose_level))
            {
                vTaskDelay(1);
            }
        }
        halt();
        on_resume();
    }
}

template <Action F1, Action F2>
void halt_if_input(OnResume<F1> on_resume, OnHalt<F2> on_halt, Verbosity verbose_level = Verbosity::Normal) noexcept
{
    halt_if_input(on_halt, on_resume, verbose_level);
}

template <Action F>
void halt_if_input(OnHalt<F> on_halt, Verbosity verbose_level = Verbosity::Normal) noexcept
{
    halt_if_input(on_halt, OnResume([] {}), verbose_level);
}

template <Action F>
void halt_if_input(OnResume<F> on_resume, Verbosity verbose_level = Verbosity::Normal) noexcept
{
    halt_if_input(OnResume([] {}), verbose_level);
}

inline void halt_if_input(Verbosity verbose_level = Verbosity::Normal) noexcept
{
    halt_if_input(OnHalt([] {}), OnResume([] {}), verbose_level);
}

}  // namespace debug_utils

#endif  // MAIN_DEBUG_UTILS_H
