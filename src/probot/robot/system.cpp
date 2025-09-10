#include <Arduino.h>
#include <probot/robot/system.hpp>
#ifdef ESP32
#include <platform/esp32s3/web/driver_station_esp32.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

namespace {
  probot::robot::StateService   g_state_singleton;
  probot::io::GamepadService    g_gamepad_singleton;
#ifdef ESP32
  probot::platform::esp32::DriverStation* g_ds_ptr = nullptr;
  IPAddress g_owner_ip;
  const char* g_ds_password = nullptr;
#endif
}

namespace probot::robot {
  StateService& state(){ return g_state_singleton; }
}
namespace probot::io {
  GamepadService& gamepad(){ return g_gamepad_singleton; }
}

namespace probot::platform {
  void driver_station_task(void*){
  #ifdef ESP32
    for(;;){ if (g_ds_ptr) g_ds_ptr->handleClient(); vTaskDelay(pdMS_TO_TICKS(1)); }
  #else
    (void)0;
  #endif
  }

  void set_driver_station_password(const char* password){
  #ifdef ESP32
    g_ds_password = password;
  #else
    (void)password;
  #endif
  }

  void start_driver_station(){
  #ifdef ESP32
    static probot::platform::esp32::DriverStation ds(g_state_singleton, g_gamepad_singleton, g_ds_password);
    g_ds_ptr = &ds;
    ds.begin();
    static TaskHandle_t h=nullptr;
    xTaskCreatePinnedToCore(driver_station_task, "ds_http", 4096, NULL, 1, &h, 0);
  #endif
  }
} 