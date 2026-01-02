#pragma once
#include <stdint.h>

#ifdef ESP32
#include <esp_task_wdt.h>
#include <esp_idf_version.h>
#include <esp_err.h>
#endif

namespace probot {
  inline void wdt_init_no_idle(uint32_t timeout_s = 3, bool panic_on = true){
#ifdef ESP32
    esp_task_wdt_deinit();
#if ESP_IDF_VERSION_MAJOR >= 5
    esp_task_wdt_config_t cfg = {
      .timeout_ms     = timeout_s * 1000,
      .idle_core_mask = 0,
      .trigger_panic  = panic_on
    };
    ESP_ERROR_CHECK( esp_task_wdt_init(&cfg) );
#else
    (void)timeout_s; (void)panic_on;
#endif
#else
    (void)timeout_s; (void)panic_on;
#endif
  }
}
