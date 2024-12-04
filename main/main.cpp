/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"

#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led_strip.h>

#include <stdio.h>

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
inline constexpr auto blink_gpio = static_cast<gpio_num_t>(CONFIG_BLINK_GPIO);

static uint8_t s_led_state = 0;

#ifdef CONFIG_BLINK_LED_RMT

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state)
    {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    }
    else
    {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = blink_gpio,
        .max_leds = 1,  // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1'000 * 1'000,  // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(blink_gpio, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(blink_gpio);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(blink_gpio, GPIO_MODE_OUTPUT);
}

#endif

#if CONFIG_BLINK_PATTERN_MORSE

inline constexpr char text[] = CONFIG_BLINK_PATTERN_MORSE_TEXT;

enum class MorseElement
{
    Dot,        // dot/dit - '1'
    Dash,       // dash/dah - '111'
    MarkSep,    // intra-character gap (between dits and dahs within a character) - '0'
    LetterSep,  // gap between characters - '000'
    WordSep,    // gap between words - '0000000'
};

constexpr size_t ticks(MorseElement m) noexcept
{
    switch (m)
    {
    case MorseElement::Dot:
        return 1;
    case MorseElement::Dash:
        return 3;
    case MorseElement::MarkSep:
        return 1;
    case MorseElement::LetterSep:
        return 3;
    case MorseElement::WordSep:
        return 7;
    }

    return 0;
}

constexpr uint8_t state(MorseElement m) noexcept
{
    switch (m)
    {
    case MorseElement::Dot:
    case MorseElement::Dash:
        return 1;

    case MorseElement::MarkSep:
    case MorseElement::LetterSep:
    case MorseElement::WordSep:
        return 0;
    }

    return 0;
}

struct MorseSymbol
{
    static constexpr auto max_elements = 20UZ;
    MorseElement elements[max_elements];
    size_t used_elements = 0;

    constexpr auto begin() noexcept { return elements; }
    constexpr auto begin() const noexcept { return elements; }
    constexpr auto cbegin() const noexcept { return begin(); }

    constexpr auto end() noexcept { return elements + used_elements; }
    constexpr auto end() const noexcept { return elements + used_elements; }
    constexpr auto cend() const noexcept { return end(); }

    constexpr auto size() const noexcept { return used_elements; }

    using value_type = MorseElement;
    using reference = MorseElement &;
    using const_reference = const MorseElement &;
    using pointer = MorseElement *;
    using const_pointer = const MorseElement *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = decltype(MorseSymbol::used_elements);

    constexpr MorseSymbol() noexcept = default;

    template <size_t N>
        requires (N <= max_elements)
    constexpr MorseSymbol(const MorseElement (&elems)[N]) noexcept : elements{}
                                                                   , used_elements(N)
    {
        for (auto i = 0ZU; i < N; ++i)
        {
            elements[i] = elems[i];
        }
    }

    template <size_t N>
        requires (N * 2 <= max_elements)
    static constexpr MorseSymbol from_elements(const MorseElement (&elems)[N], bool word_end = false) noexcept
    {
        MorseElement expanded[N * 2];
        for (auto i = 0UZ; i < N; ++i)
        {
            const auto ei = i * 2;
            expanded[ei] = elems[i];
            expanded[ei + 1] = MorseElement::MarkSep;
        }
        expanded[(N * 2) - 1] = word_end ? MorseElement::WordSep : MorseElement::LetterSep;
        return {expanded};
    }

    static constexpr MorseSymbol from_char(char c, bool word_end = false) noexcept
    {
        // clang-format off
        switch (c)
        {
        case 'a':
        case 'A':
            return from_elements({MorseElement::Dot, MorseElement::Dash}, word_end);

        case 'b':
        case 'B':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot}, word_end);

        case 'c':
        case 'C':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot}, word_end);

        case 'd':
        case 'D':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dot}, word_end);

        case 'e':
        case 'E':
            return from_elements({MorseElement::Dot}, word_end);

        case 'f':
        case 'F':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot}, word_end);

        case 'g':
        case 'G':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dot}, word_end);

        case 'h':
        case 'H':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot}, word_end);

        case 'i':
        case 'I':
            return from_elements({MorseElement::Dot, MorseElement::Dot}, word_end);

        case 'j':
        case 'J':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash}, word_end);

        case 'k':
        case 'K':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dash}, word_end);

        case 'l':
        case 'L':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot}, word_end);

        case 'm':
        case 'M':
            return from_elements({MorseElement::Dash, MorseElement::Dash}, word_end);

        case 'n':
        case 'N':
            return from_elements({MorseElement::Dash, MorseElement::Dot}, word_end);

        case 'o':
        case 'O':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dash}, word_end);

        case 'p':
        case 'P':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot}, word_end);

        case 'q':
        case 'Q':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash}, word_end);

        case 'r':
        case 'R':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dot}, word_end);

        case 's':
        case 'S':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dot}, word_end);

        case 't':
        case 'T':
            return from_elements({MorseElement::Dash}, word_end);

        case 'u':
        case 'U':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dash}, word_end);

        case 'v':
        case 'V':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash}, word_end);

        case 'w':
        case 'W':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dash}, word_end);

        case 'x':
        case 'X':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash}, word_end);

        case 'y':
        case 'Y':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash}, word_end);

        case 'z':
        case 'Z':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot}, word_end);

        case '0':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash}, word_end);

        case '1':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash}, word_end);

        case '2':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash}, word_end);

        case '3':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash}, word_end);

        case '4':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash}, word_end);

        case '5':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot}, word_end);

        case '6':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot}, word_end);

        case '7':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot}, word_end);

        case '8':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot}, word_end);

        case '9':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot}, word_end);

        case '.':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash}, word_end);

        case ',':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash}, word_end);

        case '?':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot}, word_end);

        case '\'':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot}, word_end);

        case '!':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash}, word_end);

        case '/':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot}, word_end);

        case '(':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot}, word_end);

        case ')':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash}, word_end);

        case '&':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot}, word_end);

        case ':':
            return from_elements({MorseElement::Dash, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot}, word_end);

        case ';':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot}, word_end);

        case '=':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash}, word_end);

        case '+':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot}, word_end);

        case '-':
            return from_elements({MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash}, word_end);

        case '_':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash}, word_end);

        case '"':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot}, word_end);

        case '$':
            return from_elements({MorseElement::Dot, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot, MorseElement::Dot, MorseElement::Dash}, word_end);

        case '@':
            return from_elements({MorseElement::Dot, MorseElement::Dash, MorseElement::Dash, MorseElement::Dot, MorseElement::Dash, MorseElement::Dot}, word_end);

        default:
            return {{MorseElement::MarkSep}};  // PH error
        }
        // clang-format on
    }
};

constexpr bool is_word_end(char c)
{
    switch (c)
    {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
    case '\0':
        return true;

    default:
        return false;
    }
}

[[noreturn]]
void morse_loop()
{
    auto pos = text;
    static constexpr TickType_t delay = CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS;

    while (true)
    {
        auto morse = MorseSymbol::from_char(*pos, is_word_end(*(pos + 1)));
        for (auto elem : morse)
        {
            s_led_state = state(elem);
            ESP_LOGI("morse", "Turning the LED %s!", s_led_state ? "ON" : "OFF");
            blink_led();
            vTaskDelay(delay * ticks(elem));
        }

        // Next char, infinite cycle
        if (*++pos == '\0')
        {
            pos = text;
        }
    }
}
#endif

extern "C" void app_main(void)
{
    /* Configure the peripheral according to the LED type */
    configure_led();

#if CONFIG_BLINK_PATTERN_MORSE
    morse_loop();
#else
    while (true)
    {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
#endif
}
