#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <probot/core/core_config.hpp>
#include <probot/core/wdt.hpp>
#include <probot/robot/system.hpp>
#include <probot/devices/leds/builtin.hpp>

namespace probot {
  void runtime_setup();
}

// User hooks (provided by sketch)
void robotInit();
void robotEnd();
void teleopInit();
void teleopLoop();
void autonomousInit();
void autonomousLoop();

namespace probot {
  namespace detail {
    struct RuntimeState {
      TaskHandle_t hUser = nullptr;
      TaskHandle_t hAuto = nullptr;
      TaskHandle_t hTeleop = nullptr;
      TaskHandle_t hInit = nullptr;
      TaskHandle_t hEnd = nullptr;
      volatile uint32_t auto_start_ms = 0;
      volatile uint32_t end_start_ms = 0;
    };

    inline RuntimeState g_state{};

    inline void autonomousWorker(void*){
      uint32_t now = millis();
      __atomic_store_n(&g_state.auto_start_ms, now, __ATOMIC_SEQ_CST);
      probot::robot::state().setAutoStartMs(now, now);
      ::autonomousInit();
      for(;;){ ::autonomousLoop(); vTaskDelay(pdMS_TO_TICKS(20)); }
    }

    inline void teleopWorker(void*){
      ::teleopInit();
      for(;;){ ::teleopLoop(); vTaskDelay(pdMS_TO_TICKS(20)); }
    }

    inline void robotInitWorker(void*){
      ::robotInit();
      g_state.hInit = nullptr;
      vTaskDelete(NULL);
    }

    inline void robotEndWorker(void*){
      __atomic_store_n(&g_state.end_start_ms, millis(), __ATOMIC_SEQ_CST);
      ::robotEnd();
      __atomic_store_n(&g_state.end_start_ms, 0u, __ATOMIC_SEQ_CST);
      g_state.hEnd = nullptr;
      vTaskDelete(NULL);
    }

    inline void stopAutonomous(){
      auto& s = g_state;
      if (s.hAuto){ vTaskDelete(s.hAuto); s.hAuto = nullptr; }
      __atomic_store_n(&s.auto_start_ms, 0u, __ATOMIC_SEQ_CST);
      probot::robot::state().setAutoStartMs(millis(), 0u);
    }

    inline void stopTeleop(){
      auto& s = g_state;
      if (s.hTeleop){ vTaskDelete(s.hTeleop); s.hTeleop = nullptr; }
    }

    inline void stopInit(){
      auto& s = g_state;
      if (s.hInit){ vTaskDelete(s.hInit); s.hInit = nullptr; }
    }

    inline void startAutonomous(){
      auto& s = g_state;
      stopTeleop();
      stopAutonomous();
      xTaskCreatePinnedToCore(autonomousWorker, "auto", STACK_USER, NULL, PRIO_USER, &s.hAuto, CORE_CTRL);
    }

    inline void startTeleop(){
      auto& s = g_state;
      stopAutonomous();
      stopTeleop();
      xTaskCreatePinnedToCore(teleopWorker, "teleop", STACK_USER, NULL, PRIO_USER, &s.hTeleop, CORE_CTRL);
    }

    inline void startInit(){
      auto& s = g_state;
      if (s.hInit) return;
      xTaskCreatePinnedToCore(robotInitWorker, "init", STACK_USER, NULL, PRIO_USER, &s.hInit, CORE_CTRL);
    }

    inline void startRobotEnd(){
      auto& s = g_state;
      if (s.hEnd) return;
      xTaskCreatePinnedToCore(robotEndWorker, "end", STACK_USER, NULL, PRIO_USER, &s.hEnd, CORE_CTRL);
    }

    inline void stopRobotEnd(){
      auto& s = g_state;
      if (s.hEnd){ vTaskDelete(s.hEnd); s.hEnd = nullptr; }
      __atomic_store_n(&s.end_start_ms, 0u, __ATOMIC_SEQ_CST);
    }

    inline void updateLed(){
      auto s = probot::robot::state().read();
      static bool on = false;
      on = !on;
      static uint32_t deadlineMissTime = 0;

      // Show deadline miss for 2 seconds (4 blinks) then auto-clear
      if (s.deadlineMiss){
        if (deadlineMissTime == 0) deadlineMissTime = millis();
        if (millis() - deadlineMissTime > 2000){
          probot::robot::state().setDeadlineMiss(millis(), false);
          deadlineMissTime = 0;
        } else {
          // Blink red
          if (on) builtinled::setColor(255,0,0);
          else builtinled::setColor(0,0,0);
          return;
        }
      } else {
        deadlineMissTime = 0;
      }

      switch (s.phase){
        case probot::robot::Phase::NOT_INIT:
          if (s.clientCount > 0){ if (on) builtinled::setColor(0,0,255); else builtinled::setColor(0,0,0); }
          else { builtinled::setColor(0,0,255); }
          break;
        case probot::robot::Phase::INITED:
          builtinled::setColor(255,255,0);
          break;
        case probot::robot::Phase::AUTONOMOUS:
          if (on) builtinled::setColor(255,128,0); else builtinled::setColor(0,0,0);
          break;
        case probot::robot::Phase::TELEOP:
          if (on) builtinled::setColor(0,255,0); else builtinled::setColor(0,0,0);
          break;
      }
    }

    inline void userLoopTask(){
      using probot::robot::Status;
      using probot::robot::Phase;
      Status lastStatus = Status::STOP;
      int32_t autoLen = 0;
      uint32_t lastLed = 0;
      for(;;){
        auto s = probot::robot::state().read();
        uint32_t now = millis();

        bool endRunning = (detail::g_state.hEnd != nullptr);
        uint32_t endStart = __atomic_load_n(&detail::g_state.end_start_ms, __ATOMIC_SEQ_CST);
        if (endRunning && endStart != 0u && (int32_t)(now - endStart) >= (int32_t)END_KILL_TIMEOUT_MS){
          stopRobotEnd();
          endRunning = false;
        }

        if (s.status != lastStatus){
          if (s.status == Status::INIT){
            stopAutonomous();
            stopTeleop();
            stopRobotEnd();
            startInit();
            probot::robot::state().setPhase(now, Phase::INITED);
          } else if (s.status == Status::START){
            stopInit();
            stopRobotEnd();
            if (s.autonomousEnabled){
              autoLen = s.autoPeriodSeconds;
              __atomic_store_n(&detail::g_state.auto_start_ms, 0u, __ATOMIC_SEQ_CST);
              probot::robot::state().setAutoStartMs(now, 0u);
              probot::robot::state().setPhase(now, Phase::AUTONOMOUS);
              startAutonomous();
            } else {
              probot::robot::state().setPhase(now, Phase::TELEOP);
              startTeleop();
            }
          } else if (s.status == Status::STOP){
            stopInit();
            stopAutonomous();
            stopTeleop();
            startRobotEnd();
            probot::robot::state().setPhase(now, Phase::NOT_INIT);
          }
          lastStatus = s.status;
        }

        if (s.status == Status::START && s.autonomousEnabled){
          uint32_t autoStart = __atomic_load_n(&detail::g_state.auto_start_ms, __ATOMIC_SEQ_CST);
          if (autoLen > 0 && autoStart != 0u && (int32_t)(now - autoStart) >= autoLen * 1000){
            stopAutonomous();
            probot::robot::state().setAutonomous(now, false);
            probot::robot::state().setPhase(now, Phase::TELEOP);
            startTeleop();
          }
        }

        if (s.status == Status::START && s.phase == Phase::AUTONOMOUS && !s.autonomousEnabled){
          stopAutonomous();
          probot::robot::state().setPhase(now, Phase::TELEOP);
          startTeleop();
        }

        if (now - lastLed >= 500){
          lastLed = now;
          updateLed();
        }
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }
  } // namespace detail

  inline void runtime_setup(){
    Serial.begin(115200);
    delay(200);
    Serial.println("\n[Probot] Core0=FREE, Core1=CTRL(high)+USER(low)");

    wdt_init_no_idle(3, true);

#ifdef ESP32
    probot::driverstation::start_driver_station();
#endif

    auto& s = detail::g_state;
    xTaskCreatePinnedToCore([](void*){ detail::userLoopTask(); }, "user", STACK_USER, NULL, PRIO_USER, &s.hUser, CORE_CTRL);
  }

} // namespace probot

#if defined(ARDUINO)
__attribute__((used)) inline void setup(){
  probot::runtime_setup();
}

__attribute__((used)) inline void loop(){
  delay(100);
}
#endif
