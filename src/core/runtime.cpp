#include <Arduino.h>
#include <probot/core/runtime.hpp>
#include <probot/core/core_config.hpp>
#include <probot/core/wdt.hpp>
#include <probot/core/scheduler.hpp>
#include <probot/io/input.hpp>
#include <probot/robot/system.hpp>
#include <probot/devices/leds/builtin.hpp>

#if PROBOT_WITH_DS && defined(ESP32)
#ifndef PROBOT_WIFI_AP_PASSWORD
extern const char* PROBOT__DS_PASSWORD_REQUIRED;
#endif
#endif

namespace probot {
  static TaskHandle_t hUi=nullptr, hCtrl=nullptr, hUser=nullptr;
  static TaskHandle_t hAuto=nullptr, hTeleop=nullptr;

  extern void uiTask(void*);

  static void autonomousWorker(void*){
    for(;;){ ::autonomousLoop(); vTaskDelay(pdMS_TO_TICKS(1)); }
  }
  static void teleopWorker(void*){
    for(;;){ ::teleopLoop(); vTaskDelay(pdMS_TO_TICKS(1)); }
  }

  static void stopAutonomous(){ if (hAuto){ vTaskDelete(hAuto); hAuto=nullptr; } }
  static void stopTeleop(){ if (hTeleop){ vTaskDelete(hTeleop); hTeleop=nullptr; } }
  static void startAutonomous(){ stopTeleop(); stopAutonomous(); xTaskCreatePinnedToCore(autonomousWorker, "auto", STACK_USER, NULL, PRIO_USER, &hAuto, CORE_CTRL); }
  static void startTeleop(){ stopAutonomous(); stopTeleop(); xTaskCreatePinnedToCore(teleopWorker, "teleop", STACK_USER, NULL, PRIO_USER, &hTeleop, CORE_CTRL); }

  static void updateLed(){
    auto s = probot::robot::state().read();
    static bool on=false; on=!on;
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

  static void userLoopTask(){
    using probot::robot::Status; using probot::robot::Phase;
    bool inited=false; bool inTeleop=false; bool inAuto=false; uint32_t autoStartWallMs=0; int32_t autoLen=0; uint32_t lastLed=0;
    for(;;){
      auto s = probot::robot::state().read();
      if (!inited && s.status == Status::INIT){ ::robotInit(); inited=true; inTeleop=false; inAuto=false; probot::robot::state().setPhase(millis(), Phase::INITED); }
      if (s.status == Status::START){
        if (s.autonomousEnabled){
          if (!inAuto){ ::autonomousInit(); inAuto=true; inTeleop=false; autoStartWallMs = millis(); autoLen = s.autoPeriodSeconds; probot::robot::state().setPhase(millis(), Phase::AUTONOMOUS); startAutonomous(); }
          uint32_t now = millis();
          if (autoLen>0 && (int32_t)(now - autoStartWallMs) >= autoLen*1000){
            stopAutonomous(); ::teleopInit(); inTeleop=true; inAuto=false; probot::robot::state().setPhase(millis(), Phase::TELEOP); probot::robot::state().setAutonomous(millis(), false); startTeleop();
          }
        } else {
          if (!inTeleop){ ::teleopInit(); inTeleop=true; inAuto=false; probot::robot::state().setPhase(millis(), Phase::TELEOP); startTeleop(); }
        }
      } else if (s.status == Status::STOP){
        if (inited){ stopAutonomous(); stopTeleop(); ::robotEnd(); inited=false; inTeleop=false; inAuto=false; probot::robot::state().setPhase(millis(), Phase::NOT_INIT); }
      }
      uint32_t now=millis(); if (now - lastLed >= 500){ lastLed = now; updateLed(); }
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  void runtime_setup(){
    Serial.begin(115200);
    delay(200);
    Serial.println("\n[PROBOT] Core0=UI+INPUT, Core1=CTRL(high)+USER(low)");

    wdt_init_no_idle(3, true);
    control::init(8);

#if PROBOT_WITH_DS && defined(ESP32)
    probot::platform::start_driver_station();
#endif

    xTaskCreatePinnedToCore(uiTask,                 "ui",   STACK_UI,   NULL, PRIO_UI,   &hUi,   CORE_UI);
    xTaskCreatePinnedToCore(control::schedulerTask, "ctrl", STACK_CTRL, NULL, PRIO_CTRL, &hCtrl, CORE_CTRL);
    xTaskCreatePinnedToCore([](void*){ userLoopTask(); }, "user", STACK_USER, NULL, PRIO_USER, &hUser, CORE_CTRL);
  }
} 