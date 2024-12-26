#ifndef MAIN_PERIODIC_CALLER_H
#define MAIN_PERIODIC_CALLER_H

#include <chrono>

#include <esp_timer.h>

class PeriodicCaller
{
public:
    /**
     * @brief Construct a new Periodic Caller.
     *
     * @param callback A function of type `void (void* arg)` to be called.
     * @param args Pointer to opaque user-specific data.
     */
    PeriodicCaller(esp_timer_cb_t callback, void *args) noexcept;
    ~PeriodicCaller();
    PeriodicCaller(const PeriodicCaller &) noexcept = delete;
    PeriodicCaller(PeriodicCaller &&) noexcept = delete;
    PeriodicCaller &operator=(const PeriodicCaller &) noexcept = delete;
    PeriodicCaller &operator=(PeriodicCaller &&) noexcept = delete;

    /**
     * @brief Start a periodic timer
     * This function will start the caller which will trigger every `period`.
     *
     * @param period timer period.
     */
    void start(const std::chrono::microseconds &period) noexcept;

    template <typename Rep, typename Ratio>
    auto start(const std::chrono::duration<Rep, Ratio> &period) noexcept
    {
        return start(duration_cast<std::chrono::microseconds>(period));
    }

    /**
     * @brief Stop the timer
     *
     */
    void stop() noexcept;

    inline esp_timer_handle_t get_timer() const noexcept { return m_loop_timer; };

private:
    esp_timer_handle_t m_loop_timer;
};

#endif  // MAIN_PERIODIC_CALLER_H
