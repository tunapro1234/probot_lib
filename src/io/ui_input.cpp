#include <Arduino.h>
#include <probot/io/input.hpp>
#include <probot/io/time.hpp>
#include <probot/robot/system.hpp>

namespace probot {
  static UiState g_ui_buf[2];
  static volatile uint32_t g_ui_cur_idx = 0;

  static InputState g_in_buf[2];
  static volatile uint32_t g_in_cur_idx = 0;

  static inline void atomic_store_u32(volatile uint32_t* p, uint32_t v){ __atomic_store_n(p, v, __ATOMIC_SEQ_CST); }
  static inline uint32_t atomic_load_u32(volatile uint32_t* p){ return __atomic_load_n(p, __ATOMIC_SEQ_CST); }
  static inline int16_t clamp_axis_i16(float v){
    if (v > 1.0f) v = 1.0f;
    else if (v < -1.0f) v = -1.0f;
    return static_cast<int16_t>(v * 32767.0f);
  }

  UiState read_ui_snapshot(){
    uint32_t idx = atomic_load_u32(&g_ui_cur_idx);
    return g_ui_buf[idx];
  }

  InputState read_input_snapshot(){
    uint32_t idx = atomic_load_u32(&g_in_cur_idx);
    return g_in_buf[idx];
  }

  void uiTask(void *){
    Serial.printf("[UI   ] start on core %d (prio=%u)\n", xPortGetCoreID(), (unsigned)uxTaskPriorityGet(NULL));
    uint32_t ui_seq=0, in_seq=0;
    uint32_t last_ui=0, last_in=0;

    for(;;){
      uint32_t now = millis();

      if (now - last_ui >= 50){
        last_ui = now;
        uint32_t cur = atomic_load_u32(&g_ui_cur_idx);
        uint32_t w   = 1u - cur;
        g_ui_buf[w].seq = ++ui_seq;
        g_ui_buf[w].ms  = now;
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        atomic_store_u32(&g_ui_cur_idx, w);
      }

      if (now - last_in >= 10){
        last_in = now;
        uint32_t cur = atomic_load_u32(&g_in_cur_idx);
        uint32_t w   = 1u - cur;

        auto snap = probot::io::gamepad().read();
        float x = (snap.axisCount > 0) ? snap.axes[0] : 0.0f;
        float y = (snap.axisCount > 1) ? snap.axes[1] : 0.0f;
        bool btn = (snap.buttonCount > 0) ? snap.buttons[0] : false;

        g_in_buf[w].seq = ++in_seq;
        g_in_buf[w].ms  = now;
        g_in_buf[w].x   = clamp_axis_i16(x);
        g_in_buf[w].y   = clamp_axis_i16(y);
        g_in_buf[w].btn = btn ? 1 : 0;
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        atomic_store_u32(&g_in_cur_idx, w);
      }

      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
} 
