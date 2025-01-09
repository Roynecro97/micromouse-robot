/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <maze_solver/maze_samples/small_8x8.h>

#include "../hexdump.h"
#include "../led_loop_utils.h"

#include <gtest/gtest.h>

#include <sdkconfig.h>

#include <cstddef>
#include <cstdint>

#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led_strip.h>

#include "hack.h"

LOAD_TEST_FILE(physical_size_tests);
LOAD_TEST_FILE(strongly_typed_tests);
LOAD_TEST_FILE(type_utils_tests);
LOAD_TEST_FILE(value_range_tests);

LOAD_TEST_FILE(cell_tests);
LOAD_TEST_FILE(direction_tests);
LOAD_TEST_FILE(maze_tests);

void run_tests()
{
    int argc = 1;
    char prog_name[] = "test_image";
    char color_flag[] = "--gtest_color=yes";
    char *argv[] = {prog_name, color_flag, nullptr};
    ESP_LOGI("googletest", "Running gtest...");
    ::testing::InitGoogleTest(&argc, argv);
    const int retcode = RUN_ALL_TESTS();
    if (retcode == 0)
    {
        ESP_LOGI("googletest", "All tests passed!");
    }
    else
    {
        ESP_LOGE("googletest", "Error in tests (exitcode: %d)", retcode);
    }

    ESP_LOGI("preview", "Running hexdump on a maze multiple times");
    micromouse::hexdump(micromouse::mazes::small_8x8);
    micromouse::hexdump_log_e(micromouse::mazes::small_8x8);
    micromouse::hexdump_log_w(micromouse::mazes::small_8x8);
    micromouse::hexdump_log_i(micromouse::mazes::small_8x8);
    micromouse::HexDumperE{}(micromouse::mazes::small_8x8);
}

extern "C" void app_main(void)
{
    led::infinite_loop([] { run_tests(); });
}
