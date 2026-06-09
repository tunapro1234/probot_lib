#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <probot/core/core_config.hpp>
#include <probot/core/wdt.hpp>

#ifndef PROBOT_DS_TIMEOUT_MS
#define PROBOT_DS_TIMEOUT_MS 10000
#endif

// On DS activity timeout:
//   1  -> set Status::STOP (kills teleop/auto tasks, user code stops)   — default, safe
//   0  -> only forceDisconnect (release owner, zero gamepad); user loops keep running
// Teams that want auto-recovery when the link returns without a full
// robot restart can set this to 0 in their sketch.
#ifndef PROBOT_DS_TIMEOUT_FORCE_STOP
#define PROBOT_DS_TIMEOUT_FORCE_STOP 1
#endif
#include <probot/robot/system.hpp>
#include <probot/telemetry/telemetry.hpp>
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
    // Task handles are owned EXCLUSIVELY by the sysloop task: workers
    // never touch them (a worker nulling its own handle while sysloop
    // reads it was a use-after-free). Completion is signalled through
    // the atomic flags below; finished workers park in vTaskSuspend and
    // wait to be reaped.
    struct RuntimeState {
      TaskHandle_t hSysloop = nullptr;
      TaskHandle_t hAuto = nullptr;
      TaskHandle_t hTeleop = nullptr;
      TaskHandle_t hInit = nullptr;
      TaskHandle_t hEnd = nullptr;
      volatile uint32_t auto_start_ms = 0;
      volatile uint32_t end_start_ms = 0;
      volatile uint32_t stop_req = 0;       // loop workers exit at the next boundary
      volatile uint32_t auto_parked = 0;
      volatile uint32_t teleop_parked = 0;
      volatile uint32_t init_done = 0;
      volatile uint32_t end_done = 0;
    };

    inline RuntimeState g_state{};

    inline void autonomousWorker(void*){
      uint32_t now = millis();
      __atomic_store_n(&g_state.auto_start_ms, now, __ATOMIC_SEQ_CST);
      __atomic_store_n(&probot::robot::g_loop_heartbeat_ms, now, __ATOMIC_SEQ_CST);
      probot::robot::state().setAutoStartMs(now, now);
      ::autonomousInit();
      while (!__atomic_load_n(&g_state.stop_req, __ATOMIC_SEQ_CST)){
        ::autonomousLoop();
        __atomic_store_n(&probot::robot::g_loop_heartbeat_ms, millis(), __ATOMIC_SEQ_CST);
        vTaskDelay(pdMS_TO_TICKS(20));
      }
      __atomic_store_n(&g_state.auto_parked, 1u, __ATOMIC_SEQ_CST);
      vTaskSuspend(NULL);
    }

    inline void teleopWorker(void*){
      __atomic_store_n(&probot::robot::g_loop_heartbeat_ms, millis(), __ATOMIC_SEQ_CST);
      ::teleopInit();
      while (!__atomic_load_n(&g_state.stop_req, __ATOMIC_SEQ_CST)){
        ::teleopLoop();
        __atomic_store_n(&probot::robot::g_loop_heartbeat_ms, millis(), __ATOMIC_SEQ_CST);
        vTaskDelay(pdMS_TO_TICKS(20));
      }
      __atomic_store_n(&g_state.teleop_parked, 1u, __ATOMIC_SEQ_CST);
      vTaskSuspend(NULL);
    }

    inline void robotInitWorker(void*){
      ::robotInit();
      __atomic_store_n(&g_state.init_done, 1u, __ATOMIC_SEQ_CST);
      vTaskSuspend(NULL);
    }

    inline void robotEndWorker(void*){
      __atomic_store_n(&g_state.end_start_ms, millis(), __ATOMIC_SEQ_CST);
      ::robotEnd();
      __atomic_store_n(&g_state.end_done, 1u, __ATOMIC_SEQ_CST);
      vTaskSuspend(NULL);
    }

    // Wait for a worker to reach its park point. Returns false on
    // timeout (user code is blocked — caller hard-kills).
    inline bool waitFlag(volatile uint32_t* flag, uint32_t timeout_ms){
      uint32_t t0 = millis();
      while (!__atomic_load_n(flag, __ATOMIC_SEQ_CST)){
        if ((uint32_t)(millis() - t0) >= timeout_ms) return false;
        vTaskDelay(pdMS_TO_TICKS(5));
      }
      return true;
    }

    inline void taskCreateFailed(const char* what){
      probot::telemetry::printf("!! %s task create FAILED\n", what);
      probot::robot::state().setDeadlineMiss(millis(), true);
    }

    inline void stopAutonomous(){
      auto& s = g_state;
      if (s.hAuto){
        // Cooperative stop: let the worker finish its current loop
        // iteration (~3 periods grace) so it isn't killed mid-Serial/
        // Wire transaction; hard-kill only if the user code is blocked.
        __atomic_store_n(&s.stop_req, 1u, __ATOMIC_SEQ_CST);
        waitFlag(&s.auto_parked, 60);
        vTaskDelete(s.hAuto);
        s.hAuto = nullptr;
        __atomic_store_n(&s.stop_req, 0u, __ATOMIC_SEQ_CST);
        __atomic_store_n(&s.auto_parked, 0u, __ATOMIC_SEQ_CST);
      }
      __atomic_store_n(&s.auto_start_ms, 0u, __ATOMIC_SEQ_CST);
      __atomic_store_n(&probot::robot::g_loop_heartbeat_ms, 0u, __ATOMIC_SEQ_CST);
      probot::robot::state().setAutoStartMs(millis(), 0u);
    }

    inline void stopTeleop(){
      auto& s = g_state;
      if (s.hTeleop){
        __atomic_store_n(&s.stop_req, 1u, __ATOMIC_SEQ_CST);
        waitFlag(&s.teleop_parked, 60);
        vTaskDelete(s.hTeleop);
        s.hTeleop = nullptr;
        __atomic_store_n(&s.stop_req, 0u, __ATOMIC_SEQ_CST);
        __atomic_store_n(&s.teleop_parked, 0u, __ATOMIC_SEQ_CST);
      }
      __atomic_store_n(&probot::robot::g_loop_heartbeat_ms, 0u, __ATOMIC_SEQ_CST);
    }

    inline void stopInit(){
      auto& s = g_state;
      if (s.hInit){ vTaskDelete(s.hInit); s.hInit = nullptr; }
      __atomic_store_n(&s.init_done, 0u, __ATOMIC_SEQ_CST);
    }

    inline void startAutonomous(){
      auto& s = g_state;
      stopTeleop();
      stopAutonomous();
      __atomic_store_n(&s.auto_parked, 0u, __ATOMIC_SEQ_CST);
      if (xTaskCreatePinnedToCore(autonomousWorker, "auto", STACK_USER, NULL, PRIO_USER, &s.hAuto, CORE_CTRL) != pdPASS){
        s.hAuto = nullptr;
        taskCreateFailed("autonomous");
      }
    }

    inline void startTeleop(){
      auto& s = g_state;
      stopAutonomous();
      stopTeleop();
      __atomic_store_n(&s.teleop_parked, 0u, __ATOMIC_SEQ_CST);
      if (xTaskCreatePinnedToCore(teleopWorker, "teleop", STACK_USER, NULL, PRIO_USER, &s.hTeleop, CORE_CTRL) != pdPASS){
        s.hTeleop = nullptr;
        taskCreateFailed("teleop");
      }
    }

    inline void startInit(){
      auto& s = g_state;
      if (s.hInit) return;
      __atomic_store_n(&s.init_done, 0u, __ATOMIC_SEQ_CST);
      if (xTaskCreatePinnedToCore(robotInitWorker, "init", STACK_USER, NULL, PRIO_USER, &s.hInit, CORE_CTRL) != pdPASS){
        s.hInit = nullptr;
        taskCreateFailed("robotInit");
      }
    }

    inline void startRobotEnd(){
      auto& s = g_state;
      if (s.hEnd) return;
      __atomic_store_n(&s.end_done, 0u, __ATOMIC_SEQ_CST);
      __atomic_store_n(&s.end_start_ms, 0u, __ATOMIC_SEQ_CST);
      if (xTaskCreatePinnedToCore(robotEndWorker, "end", STACK_USER, NULL, PRIO_USER, &s.hEnd, CORE_CTRL) != pdPASS){
        s.hEnd = nullptr;
        taskCreateFailed("robotEnd");
      }
    }

    inline void stopRobotEnd(){
      auto& s = g_state;
      if (s.hEnd){ vTaskDelete(s.hEnd); s.hEnd = nullptr; }
      __atomic_store_n(&s.end_start_ms, 0u, __ATOMIC_SEQ_CST);
      __atomic_store_n(&s.end_done, 0u, __ATOMIC_SEQ_CST);
    }

    inline void updateLed(){
      auto s = probot::robot::state().read();
      static bool on = false;
      on = !on;
      static uint32_t dmLedTime = 0;

      if (s.deadlineMiss){
        if (dmLedTime == 0) dmLedTime = millis();
        if (millis() - dmLedTime > 3000){
          probot::robot::state().setDeadlineMiss(millis(), false);
          dmLedTime = 0;
        } else {
          if (on) builtinled::setColor(255,0,0);
          else builtinled::setColor(0,0,0);
          return;
        }
      } else {
        dmLedTime = 0;
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

    inline void sysloopTask(){
      using probot::robot::Status;
      using probot::robot::Phase;
#ifdef ESP32
      // Subscribe to the task watchdog — without at least one
      // subscribed task the TWDT never fires and a wedged sysloop
      // (the task that supervises everything else) goes unnoticed.
      esp_task_wdt_add(NULL);
#endif
      Status lastStatus = Status::STOP;
      int32_t autoLen = 0;
      uint32_t lastLed = 0;
      for(;;){
#ifdef ESP32
        esp_task_wdt_reset();
#endif
        auto s = probot::robot::state().read();
        uint32_t now = millis();

        // Reap finished init/end workers (they park in vTaskSuspend).
        // INITED is reported only when robotInit() actually completed —
        // pressing Start mid-init no longer runs teleop on
        // half-initialized hardware.
        if (detail::g_state.hInit &&
            __atomic_load_n(&detail::g_state.init_done, __ATOMIC_SEQ_CST)){
          stopInit();
          probot::robot::state().setPhase(now, Phase::INITED);
        }
        if (detail::g_state.hEnd &&
            __atomic_load_n(&detail::g_state.end_done, __ATOMIC_SEQ_CST)){
          stopRobotEnd();
        }

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

        // Deadline miss: warn on teleop, kill autonomous. The
        // !s.deadlineMiss gate keeps this one-shot per episode —
        // without it the telemetry println fires every 1 ms iteration
        // and wipes the 256-byte ring exactly when it matters most.
        {
          bool taskRunning = (s.phase == Phase::TELEOP || s.phase == Phase::AUTONOMOUS);
          uint32_t hb = __atomic_load_n(&probot::robot::g_loop_heartbeat_ms, __ATOMIC_SEQ_CST);
          if (taskRunning && !s.deadlineMiss && hb != 0 && (int32_t)(now - hb) > 2000){
            probot::robot::state().setDeadlineMiss(now, true);
            if (s.phase == Phase::AUTONOMOUS){
              probot::telemetry::println("!! DEADLINE MISS — auto blocked, switching to teleop");
              stopAutonomous();
              probot::robot::state().setAutonomous(now, false);
              probot::robot::state().setPhase(now, Phase::TELEOP);
              startTeleop();
            } else {
              probot::telemetry::println("!! DEADLINE MISS — teleop blocked");
            }
          }
        }

        // DS connection heartbeat: no activity → stop robot and/or disconnect
        {
          uint32_t dsAct = __atomic_load_n(&probot::robot::g_ds_last_activity_ms, __ATOMIC_SEQ_CST);
          if (dsAct != 0 && s.status != Status::STOP &&
              (int32_t)(now - dsAct) > (int32_t)PROBOT_DS_TIMEOUT_MS){
            Serial.printf("[SYS  ] DS timeout: no activity for %lu ms\n", (unsigned long)(now - dsAct));
#if PROBOT_DS_TIMEOUT_FORCE_STOP
            probot::telemetry::println("!! DS CONNECTION LOST — stopping robot");
            // Only set the status — the transition block above must see
            // status != lastStatus on the next iteration to actually
            // tear down teleop/auto and run robotEnd.
            probot::robot::state().setStatus(now, Status::STOP);
#else
            probot::telemetry::println("!! DS CONNECTION LOST — joystick neutral, waiting reconnect");
#endif
#ifdef ESP32
            if (probot::driverstation::detail::g_driver_station){
              probot::driverstation::detail::g_driver_station->forceDisconnect(now);
            }
#endif
            __atomic_store_n(&probot::robot::g_ds_last_activity_ms, 0u, __ATOMIC_SEQ_CST);
          }
        }

        // Expire DS owner if idle (replaces handleClient polling)
#ifdef ESP32
        if (probot::driverstation::detail::g_driver_station){
          probot::driverstation::detail::g_driver_station->expireOwnerIfIdle();
          probot::driverstation::detail::g_driver_station->processDns();
        }
#endif

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
    Serial.println("\n[Probot] Core0=DS+SYS, Core1=USER");

    wdt_init_no_idle(3, true);

#ifdef ESP32
    probot::driverstation::start_driver_station();
#endif

    auto& s = detail::g_state;
    xTaskCreatePinnedToCore([](void*){ detail::sysloopTask(); }, "sysloop", STACK_CTRL, NULL, PRIO_CTRL, &s.hSysloop, CORE_UI);
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
