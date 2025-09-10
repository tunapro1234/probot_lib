#include <probot/core/scheduler.hpp>
#include <Arduino.h>
#include <esp_idf_version.h>
#include <esp_task_wdt.h>
#include <probot/robot/system.hpp>
#include <probot/robot/state.hpp>

namespace control {
  enum CmdType : uint8_t { CMD_ATTACH=0, CMD_DETACH=1 };
  struct Cmd {
    CmdType type;
    IUpdatable* obj;
  };

  QueueHandle_t qCmd = nullptr;

  struct Slot {
    bool        in_use;
    IUpdatable* obj;
    uint32_t    next_due_ms;
    uint32_t    last_call_ms;
  };

  static const size_t MAX_SLOTS = 512;
  static Slot slots[MAX_SLOTS];
  static uint32_t g_global_period_ms = 20;

  void setGlobalPeriodMs(uint32_t period_ms){
    if (period_ms == 0) return;
    g_global_period_ms = period_ms;
  }

  void init(uint8_t queue_len){
    if (!qCmd){
      qCmd = xQueueCreate(queue_len, sizeof(Cmd));
    }
  }

  static int find_slot(IUpdatable* obj){
    for (int i=0;i<(int)MAX_SLOTS;i++){
      if (slots[i].in_use && slots[i].obj == obj) return i;
    }
    return -1;
  }
  static int find_free_slot(){
    for (int i=0;i<(int)MAX_SLOTS;i++){
      if (!slots[i].in_use) return i;
    }
    return -1;
  }

  bool attach(IUpdatable* obj){
    if (!qCmd || !obj) return false;
    Cmd c{CMD_ATTACH, obj};
    return xQueueSend(qCmd, &c, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  bool detach(IUpdatable* obj){
    if (!qCmd || !obj) return false;
    Cmd c{CMD_DETACH, obj};
    return xQueueSend(qCmd, &c, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  size_t count(){
    size_t n=0;
    for (size_t i=0;i<MAX_SLOTS;i++) if (slots[i].in_use) n++;
    return n;
  }

  void schedulerTask(void*){
#ifndef PROBOT_SCHED_NOLOG
    Serial.printf("[CMD  ] start on core %d (prio=%u)\n", xPortGetCoreID(), (unsigned)uxTaskPriorityGet(NULL));
#endif
#if ESP_IDF_VERSION_MAJOR >= 5
    esp_task_wdt_add(NULL);
#endif
    for (size_t i=0;i<MAX_SLOTS;i++){
      slots[i] = {false, nullptr, 0, 0};
    }

    uint32_t now = millis();

    for(;;){
      Cmd cmd;
      while (xQueueReceive(qCmd, &cmd, 0) == pdTRUE){
        if (cmd.type == CMD_ATTACH){
          int idx = find_slot(cmd.obj);
          if (idx < 0) idx = find_free_slot();
          if (idx >= 0){
            slots[idx].in_use = true;
            slots[idx].obj = cmd.obj;
            uint32_t t = millis();
            slots[idx].last_call_ms = t;
            slots[idx].next_due_ms  = t + g_global_period_ms;
#ifndef PROBOT_SCHED_NOLOG
            Serial.printf("[CMD  ] attach ok (idx=%d, period=%lums)\n", idx, (unsigned long)g_global_period_ms);
#endif
          } else {
#ifndef PROBOT_SCHED_NOLOG
            Serial.println("[CMD  ] attach FAILED (no free slot)");
#endif
          }
        } else {
          int idx = find_slot(cmd.obj);
          if (idx >= 0){
            slots[idx] = {false, nullptr, 0, 0};
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
      auto snap = probot::robot::state().read();
      bool allowUpdates = (snap.phase != probot::robot::Phase::NOT_INIT);
      for (size_t i=0;i<MAX_SLOTS;i++){
        if (!slots[i].in_use) continue;
        if ((int32_t)(now - slots[i].next_due_ms) >= 0){
          uint32_t dt = now - slots[i].last_call_ms;
          if (dt > g_global_period_ms + 2) deadlineMiss = true;
          if (allowUpdates){ slots[i].obj->update(now, dt); }
          slots[i].last_call_ms = now;
          uint32_t due = slots[i].next_due_ms;
          while ((int32_t)(now - due) >= 0){
            due += g_global_period_ms;
          }
          slots[i].next_due_ms = due;
        }
        if ((int32_t)(slots[i].next_due_ms - nearest_due) < 0){
          nearest_due = slots[i].next_due_ms;
        }
      }
      if (deadlineMiss){ probot::robot::state().setDeadlineMiss(now, true); }

#if ESP_IDF_VERSION_MAJOR >= 5
      esp_task_wdt_reset();
#endif

      TickType_t waitTicks = pdMS_TO_TICKS(1);
      now = millis();
      if ((int32_t)(nearest_due - now) > 0){
        waitTicks = pdMS_TO_TICKS(nearest_due - now);
      }
      if (xQueueReceive(qCmd, &cmd, waitTicks) == pdTRUE){
        if (cmd.type == CMD_ATTACH){
          int idx = find_slot(cmd.obj);
          if (idx < 0) idx = find_free_slot();
          if (idx >= 0){
            slots[idx].in_use = true;
            slots[idx].obj = cmd.obj;
            uint32_t t = millis();
            slots[idx].last_call_ms = t;
            slots[idx].next_due_ms  = t + g_global_period_ms;
#ifndef PROBOT_SCHED_NOLOG
            Serial.printf("[CMD  ] attach ok (idx=%d, period=%lums)\n", idx, (unsigned long)g_global_period_ms);
#endif
          } else {
#ifndef PROBOT_SCHED_NOLOG
            Serial.println("[CMD  ] attach FAILED (no free slot)");
#endif
          }
        } else {
          int idx = find_slot(cmd.obj);
          if (idx >= 0){
            slots[idx] = {false, nullptr, 0, 0};
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
} 