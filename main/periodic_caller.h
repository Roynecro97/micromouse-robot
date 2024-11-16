#ifndef PERIODIC_CALLER_H
#define PERIODIC_CALLER_H

#include <esp_timer.h>

class periodic_caller {
public:
    /**
     * @brief Construct a new Periodic Caller.
     *
     * @param callback A function of type `void (void* arg)` to be called.
     * @param args Pointer to opaque user-specific data.
     */
    periodic_caller(esp_timer_cb_t callback, void* args);
    ~periodic_caller();

    /**
     * @brief Start a periodic timer
     * This function will start the caller which will trigger every `period` microseconds.
     *
     * @param period timer period, in microseconds
     */
    void start(uint32_t period);

    /**
     * @brief Stop the timer
     *
     */
    void stop();

    inline esp_timer_handle_t get_timer() const { return m_loop_timer; };

private:
    esp_timer_handle_t m_loop_timer;
};

#endif
