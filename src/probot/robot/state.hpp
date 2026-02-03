#pragma once
#include <stdint.h>

namespace probot::robot {
  enum class Status : uint8_t { INIT=0, START=1, STOP=2 };
  enum class Phase  : uint8_t { NOT_INIT=0, INITED=1, AUTONOMOUS=2, TELEOP=3 };

  struct StateSnapshot {
    uint32_t ms;
    uint32_t seq;
    Status   status;
    Phase    phase;
    float    batteryVoltage;
    bool     autonomousEnabled;
    int32_t  autoPeriodSeconds;
    uint32_t autoStartMs;
    int32_t  clientCount;
    bool     deadlineMiss;
  };

  class StateService {
  public:
    StateService(){
      _cur = 0;
      StateSnapshot s{};
      s.ms=0; s.seq=0; s.status=Status::STOP; s.phase=Phase::NOT_INIT; s.batteryVoltage=0.0f;
      s.autonomousEnabled=false; s.autoPeriodSeconds=30; s.autoStartMs=0; s.clientCount=0; s.deadlineMiss=false;
      _buf[0] = s; _buf[1] = s;
    }

    void setStatus(uint32_t now_ms, Status st){ writeField(now_ms, [&](StateSnapshot& w){ w.status = st; }); }
    void setPhase(uint32_t now_ms, Phase ph){ writeField(now_ms, [&](StateSnapshot& w){ w.phase = ph; }); }
    void setBatteryVoltage(uint32_t now_ms, float v){ writeField(now_ms, [&](StateSnapshot& w){ w.batteryVoltage = v; }); }
    void setAutonomous(uint32_t now_ms, bool en){ writeField(now_ms, [&](StateSnapshot& w){ w.autonomousEnabled = en; }); }
    void setAutoPeriodSeconds(uint32_t now_ms, int32_t s){ writeField(now_ms, [&](StateSnapshot& w){ w.autoPeriodSeconds = s; }); }
    void setAutoStartMs(uint32_t now_ms, uint32_t ms){ writeField(now_ms, [&](StateSnapshot& w){ w.autoStartMs = ms; }); }
    void setClientCount(uint32_t now_ms, int32_t c){ writeField(now_ms, [&](StateSnapshot& w){ w.clientCount = c; }); }
    void setDeadlineMiss(uint32_t now_ms, bool v){ writeField(now_ms, [&](StateSnapshot& w){ w.deadlineMiss = v; }); }

    StateSnapshot read() const {
      uint32_t idx = __atomic_load_n(&_cur, __ATOMIC_SEQ_CST);
      return _buf[idx];
    }

    // Convenience getters
    bool     autonomousEnabled() const { return _buf[__atomic_load_n(&_cur, __ATOMIC_SEQ_CST)].autonomousEnabled; }
    int32_t  autoPeriodSeconds() const { return _buf[__atomic_load_n(&_cur, __ATOMIC_SEQ_CST)].autoPeriodSeconds; }
    uint32_t autoStartMs() const       { return _buf[__atomic_load_n(&_cur, __ATOMIC_SEQ_CST)].autoStartMs; }
    Status   status() const            { return _buf[__atomic_load_n(&_cur, __ATOMIC_SEQ_CST)].status; }
    Phase    phase() const             { return _buf[__atomic_load_n(&_cur, __ATOMIC_SEQ_CST)].phase; }

  private:
    void lock() const {
      uint32_t expected = 0;
      while (!__atomic_compare_exchange_n(&_write_lock, &expected, 1u, false,
                                          __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        expected = 0;
      }
    }

    void unlock() const { __atomic_store_n(&_write_lock, 0u, __ATOMIC_RELEASE); }

    template<typename Fn>
    void writeField(uint32_t now_ms, Fn fn){
      lock();
      uint32_t cur = __atomic_load_n(&_cur, __ATOMIC_SEQ_CST);
      uint32_t w   = 1u - cur;
      StateSnapshot s = _buf[cur];
      s.ms = now_ms;
      s.seq++;
      fn(s);
      __atomic_thread_fence(__ATOMIC_SEQ_CST);
      _buf[w] = s;
      __atomic_store_n(&_cur, w, __ATOMIC_SEQ_CST);
      unlock();
    }

    mutable StateSnapshot     _buf[2];
    mutable volatile uint32_t _cur;
    mutable volatile uint32_t _write_lock = 0;
  };

  inline StateService& state(){
    static StateService instance;
    return instance;
  }
}
