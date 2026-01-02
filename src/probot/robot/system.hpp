#pragma once
#include <probot/robot/state.hpp>
#include <probot/io/gamepad.hpp>

#ifdef ESP32
#include <driverstation/esp32s3/driver_station_esp32.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

namespace probot::io {
  namespace detail {
    inline GamepadService g_gamepad_singleton;
  }

  inline GamepadService& gamepad(){
    return detail::g_gamepad_singleton;
  }
}

namespace probot::driverstation {
  namespace detail {
#ifdef ESP32
    inline probot::driverstation::esp32::DriverStation* g_driver_station = nullptr;
#endif
  } // namespace detail

  inline void driver_station_task(void*){
  #ifdef ESP32
    for(;;){ if (detail::g_driver_station) detail::g_driver_station->handleClient(); vTaskDelay(pdMS_TO_TICKS(1)); }
  #else
    (void)0;
  #endif
  }

  inline void start_driver_station(){
  #ifdef ESP32
    static probot::driverstation::esp32::DriverStation ds(probot::robot::state(), probot::io::gamepad());
    detail::g_driver_station = &ds;
    ds.begin();
    static TaskHandle_t h = nullptr;
    xTaskCreatePinnedToCore(driver_station_task, "ds_http", 4096, NULL, 1, &h, 0);
  #endif
  }
}
