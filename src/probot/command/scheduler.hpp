#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <probot/command/command.hpp>
#include <probot/core/scheduler_registry.hpp>
#include <probot/robot/state.hpp>

#ifdef ESP32
#include <esp_idf_version.h>
#include <esp_task_wdt.h>
#endif

namespace probot::command::scheduler {
  namespace detail {
    enum class CmdType : uint8_t { Attach = 0, Detach = 1 };
    enum class EntryKind : uint8_t { Command = 0, Subsystem = 1 };
    struct Cmd {
      CmdType type;
      EntryKind kind;
      void* obj;
    };

    struct Slot {
      bool in_use;
      EntryKind kind;
      void* obj;
      uint32_t next_due_ms;
      uint32_t last_call_ms;
    };

    inline constexpr size_t kMaxSlots = 512;

    struct SchedulerState {
      QueueHandle_t qCmd = nullptr;
      uint32_t global_period_ms = 20;
      Slot slots[kMaxSlots] = {};
    };

    inline SchedulerState g_state{};

    inline int find_slot(SchedulerState& s, EntryKind kind, void* obj){
      for (size_t i = 0; i < kMaxSlots; ++i){
        if (s.slots[i].in_use && s.slots[i].kind == kind && s.slots[i].obj == obj) {
          return static_cast<int>(i);
        }
      }
      return -1;
    }

    inline int find_free_slot(SchedulerState& s){
      for (size_t i = 0; i < kMaxSlots; ++i){
        if (!s.slots[i].in_use) return static_cast<int>(i);
      }
      return -1;
    }
  } // namespace detail

  inline void setGlobalPeriodMs(uint32_t period_ms){
    if (period_ms == 0) return;
    detail::g_state.global_period_ms = period_ms;
  }

  inline void init(uint8_t queue_len = 8){
    auto& s = detail::g_state;
    if (!s.qCmd){
      s.qCmd = xQueueCreate(queue_len, sizeof(detail::Cmd));
    }
  }

  inline bool attach(probot::command::ICommand* obj){
    auto& s = detail::g_state;
    if (!s.qCmd || !obj) return false;
    detail::Cmd c{detail::CmdType::Attach, detail::EntryKind::Command, obj};
    return xQueueSend(s.qCmd, &c, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  inline bool attach(probot::command::ISubsystem* obj){
    auto& s = detail::g_state;
    if (!s.qCmd || !obj) return false;
    detail::Cmd c{detail::CmdType::Attach, detail::EntryKind::Subsystem, obj};
    return xQueueSend(s.qCmd, &c, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  inline bool detach(probot::command::ICommand* obj){
    auto& s = detail::g_state;
    if (!s.qCmd || !obj) return false;
    detail::Cmd c{detail::CmdType::Detach, detail::EntryKind::Command, obj};
    return xQueueSend(s.qCmd, &c, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  inline bool detach(probot::command::ISubsystem* obj){
    auto& s = detail::g_state;
    if (!s.qCmd || !obj) return false;
    detail::Cmd c{detail::CmdType::Detach, detail::EntryKind::Subsystem, obj};
    return xQueueSend(s.qCmd, &c, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  inline size_t count(){
    auto& s = detail::g_state;
    size_t n = 0;
    for (size_t i = 0; i < detail::kMaxSlots; ++i){
      if (s.slots[i].in_use) ++n;
    }
    return n;
  }

  inline void task(void*){
#ifndef PROBOT_SCHED_NOLOG
    Serial.printf("[CMD  ] start on core %d (prio=%u)\n", xPortGetCoreID(), (unsigned)uxTaskPriorityGet(NULL));
#endif
#if defined(ESP32) && ESP_IDF_VERSION_MAJOR >= 5
    esp_task_wdt_add(NULL);
#endif
    auto& s = detail::g_state;
    for (size_t i = 0; i < detail::kMaxSlots; ++i){
      s.slots[i] = {false, detail::EntryKind::Command, nullptr, 0, 0};
    }

    uint32_t now = millis();

    for(;;){
      detail::Cmd cmd;
      while (xQueueReceive(s.qCmd, &cmd, 0) == pdTRUE){
        if (cmd.type == detail::CmdType::Attach){
          int idx = detail::find_slot(s, cmd.kind, cmd.obj);
          if (idx < 0) idx = detail::find_free_slot(s);
          if (idx >= 0){
            s.slots[idx].in_use = true;
            s.slots[idx].kind = cmd.kind;
            s.slots[idx].obj = cmd.obj;
            uint32_t t = millis();
            s.slots[idx].last_call_ms = t;
            s.slots[idx].next_due_ms  = t + s.global_period_ms;
#ifndef PROBOT_SCHED_NOLOG
            Serial.printf("[CMD  ] attach ok (idx=%d, period=%lums)\n", idx, (unsigned long)s.global_period_ms);
#endif
          } else {
#ifndef PROBOT_SCHED_NOLOG
            Serial.println("[CMD  ] attach FAILED (no free slot)");
#endif
          }
        } else {
          int idx = detail::find_slot(s, cmd.kind, cmd.obj);
          if (idx >= 0){
            s.slots[idx] = {false, detail::EntryKind::Command, nullptr, 0, 0};
#ifndef PROBOT_SCHED_NOLOG
            Serial.printf("[CMD  ] detach ok (idx=%d)\n", idx);
#endif
          } else {
#ifndef PROBOT_SCHED_NOLOG
            Serial.println("[CMD  ] detach miss (not found)");
#endif
          }
        }
      }

      now = millis();
      uint32_t nearest_due = now + 1000;
      bool deadlineMiss = false;
      uint32_t maxOverrun = 0;
      auto snap = probot::robot::state().read();
      static probot::robot::Status lastStatus = snap.status;
      static probot::robot::Phase lastPhase = snap.phase;
      bool statusToStop = (lastStatus != probot::robot::Status::STOP && snap.status == probot::robot::Status::STOP);
      bool autoToTeleop = (lastPhase == probot::robot::Phase::AUTONOMOUS && snap.phase == probot::robot::Phase::TELEOP);
      if (statusToStop || autoToTeleop){
        for (size_t i = 0; i < detail::kMaxSlots; ++i){
          if (!s.slots[i].in_use || !s.slots[i].obj) continue;
          if (s.slots[i].kind == detail::EntryKind::Command){
            static_cast<probot::command::ICommand*>(s.slots[i].obj)->onSchedulerStop();
          }
        }
      }
      lastStatus = snap.status;
      lastPhase = snap.phase;
      bool allowUpdates = (snap.status != probot::robot::Status::STOP);
      for (size_t i = 0; i < detail::kMaxSlots; ++i){
        if (!s.slots[i].in_use) continue;
        if ((int32_t)(now - s.slots[i].next_due_ms) >= 0){
          uint32_t dt = now - s.slots[i].last_call_ms;
          if (dt > s.global_period_ms + 2){
            deadlineMiss = true;
            uint32_t overrun = dt - s.global_period_ms;
            if (overrun > maxOverrun) maxOverrun = overrun;
          }
          if (allowUpdates){
            if (s.slots[i].kind == detail::EntryKind::Command){
              static_cast<probot::command::ICommand*>(s.slots[i].obj)->periodic(now, dt);
            } else {
              static_cast<probot::command::ISubsystem*>(s.slots[i].obj)->periodic(now, dt);
            }
          }
          s.slots[i].last_call_ms = now;
          uint32_t due = s.slots[i].next_due_ms;
          while ((int32_t)(now - due) >= 0){
            due += s.global_period_ms;
          }
          s.slots[i].next_due_ms = due;
        }
        if ((int32_t)(s.slots[i].next_due_ms - nearest_due) < 0){
          nearest_due = s.slots[i].next_due_ms;
        }
      }
      if (deadlineMiss){
        probot::robot::state().setDeadlineMiss(now, true);
#ifndef PROBOT_SCHED_NOLOG
        Serial.printf("[SCHED] ⚠️  CONTROL LOOP OVERRUN! Period: %lums, Overrun: +%lums\n",
                      (unsigned long)s.global_period_ms, (unsigned long)maxOverrun);
#endif
      }

#if defined(ESP32) && ESP_IDF_VERSION_MAJOR >= 5
      esp_task_wdt_reset();
#endif

      TickType_t waitTicks = pdMS_TO_TICKS(1);
      now = millis();
      if ((int32_t)(nearest_due - now) > 0){
        waitTicks = pdMS_TO_TICKS(nearest_due - now);
      }
      if (xQueueReceive(s.qCmd, &cmd, waitTicks) == pdTRUE){
        if (cmd.type == detail::CmdType::Attach){
          int idx = detail::find_slot(s, cmd.kind, cmd.obj);
          if (idx < 0) idx = detail::find_free_slot(s);
          if (idx >= 0){
            s.slots[idx].in_use = true;
            s.slots[idx].kind = cmd.kind;
            s.slots[idx].obj = cmd.obj;
            uint32_t t = millis();
            s.slots[idx].last_call_ms = t;
            s.slots[idx].next_due_ms  = t + s.global_period_ms;
#ifndef PROBOT_SCHED_NOLOG
            Serial.printf("[CMD  ] attach ok (idx=%d, period=%lums)\n", idx, (unsigned long)s.global_period_ms);
#endif
          } else {
#ifndef PROBOT_SCHED_NOLOG
            Serial.println("[CMD  ] attach FAILED (no free slot)");
#endif
          }
        } else {
          int idx = detail::find_slot(s, cmd.kind, cmd.obj);
          if (idx >= 0){
            s.slots[idx] = {false, detail::EntryKind::Command, nullptr, 0, 0};
#ifndef PROBOT_SCHED_NOLOG
            Serial.printf("[CMD  ] detach ok (idx=%d)\n", idx);
#endif
          } else {
#ifndef PROBOT_SCHED_NOLOG
            Serial.println("[CMD  ] detach miss (not found)");
#endif
          }
        }
      }
    }
  }

  namespace detail {
    struct SchedulerRegistrar {
      SchedulerRegistrar(){
        probot::detail::g_scheduler_init = &probot::command::scheduler::init;
        probot::detail::g_scheduler_task = &probot::command::scheduler::task;
      }
    };

    inline SchedulerRegistrar g_scheduler_registrar{};
  } // namespace detail
} // namespace probot::command::scheduler
