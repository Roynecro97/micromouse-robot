#ifndef MAIN_HEXDUMP_H
#define MAIN_HEXDUMP_H

#include <misc_utils/hexdump.h>

#include <sdkconfig.h>

#include <esp_log.h>

namespace micromouse::inline hd
{

inline constexpr auto log_i_printf = [](const char *msg) { ESP_LOGI("hexdump", "%s", msg); };
inline constexpr auto log_w_printf = [](const char *msg) { ESP_LOGW("hexdump", "%s", msg); };
inline constexpr auto log_e_printf = [](const char *msg) { ESP_LOGE("hexdump", "%s", msg); };

inline HexDumperP<log_i_printf> hexdump_log_i;
inline HexDumperP<log_w_printf> hexdump_log_w;
inline HexDumperP<log_e_printf> hexdump_log_e;

}  // namespace micromouse::inline hd

#endif  // MAIN_HEXDUMP_H
