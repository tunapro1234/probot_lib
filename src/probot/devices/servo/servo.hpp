#pragma once
#include <stdint.h>

#if defined(ARDUINO) && defined(ESP32)
#include <Arduino.h>
#include <esp_arduino_version.h>

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "probot::devices::Servo requires arduino-esp32 core 3.x (uses the 3.x LEDC API)."
#endif

namespace probot::devices {

/**
 * Hobby servo on ESP32 LEDC hardware PWM — jitter-safe by construction.
 *
 * Why not ESP32Servo / raw analogWrite?
 *   - Servos need a 50 Hz pulse train. analogWrite() defaults to 1 kHz,
 *     and LEDC channels share timers in pairs: if a 50 Hz servo and a
 *     1 kHz motor land on the same timer, one of them silently
 *     reconfigures the other — that is the classic "servo titremesi".
 *   - This class allocates channels from the TOP of the channel range
 *     downward, while analogWrite() allocates from the bottom up, so
 *     servos and motor PWM never collide on a timer.
 *   - 14-bit resolution at 50 Hz → 1.2 µs pulse granularity (~0.1°).
 *
 * Usage:
 *   probot::devices::Servo arm;
 *   void robotInit() { arm.attach(4); }          // GPIO 4
 *   void teleopLoop() { arm.write(90); ... }     // 0-180°
 *
 * Note: signal jitter can also come from POWER, not timers. Servos must
 * be fed from their own 5-6 V supply (BEC/UBEC), never from the ESP32
 * board's regulator. WiFi TX bursts cause voltage dips that twitch
 * servos sharing a rail. Common ground is required.
 */
class Servo {
public:
  static constexpr uint32_t FREQ_HZ    = 50;
  static constexpr uint8_t  RESOLUTION = 14;            // bits
  static constexpr uint32_t PERIOD_US  = 1000000 / FREQ_HZ;
  static constexpr uint32_t DUTY_MAX   = (1u << RESOLUTION) - 1;

  // Returns false if the pin is invalid or no LEDC channel is free.
  // Each Servo instance claims its channel ONCE and keeps it for life —
  // robotInit() re-runs on every DS Init press, so re-attach must reuse
  // the same channel instead of burning a new one each time.
  bool attach(uint8_t pin, uint16_t minUs = 500, uint16_t maxUs = 2500) {
    if (_attached) detach();
    if (minUs >= maxUs) return false;
    if (_ch < 0) {
      _ch = claimChannel();
      if (_ch < 0) return false;
    }
    if (!ledcAttachChannel(pin, FREQ_HZ, RESOLUTION, (uint8_t)_ch)) {
      return false;
    }
    _pin = pin;
    _min_us = minUs;
    _max_us = maxUs;
    _attached = true;
    // No pulses until the first write() — the servo stays where it is
    // instead of jumping on boot.
    return true;
  }

  void writeMicroseconds(uint16_t us) {
    if (!_attached) return;
    if (us < _min_us) us = _min_us;
    if (us > _max_us) us = _max_us;
    _last_us = us;
    uint32_t duty = (uint32_t)((uint64_t)us * DUTY_MAX / PERIOD_US);
    ledcWrite(_pin, duty);
  }

  // angle: 0-180 degrees, mapped onto [minUs, maxUs]
  void write(float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    writeMicroseconds((uint16_t)(_min_us + (angle / 180.0f) * (_max_us - _min_us)));
  }

  uint16_t readMicroseconds() const { return _last_us; }
  bool attached() const { return _attached; }

  // Stops the pulse train (servo goes limp) and frees the pin. The
  // instance keeps its LEDC channel for the next attach().
  void detach() {
    if (!_attached) return;
    ledcDetach(_pin);
    _attached = false;
  }

private:
  // analogWrite() hands out channels from 0 upward; we hand out from the
  // top downward so servo timers (50 Hz) never pair with motor timers.
  // Only called from user hooks (single user task) — no locking needed.
  static int8_t claimChannel() {
#ifdef LEDC_CHANNELS
    static int8_t next = LEDC_CHANNELS - 1;
#else
    static int8_t next = 7;
#endif
    if (next < 0) return -1;
    return next--;
  }

  uint8_t  _pin = 255;
  int8_t   _ch = -1;
  uint16_t _min_us = 500;
  uint16_t _max_us = 2500;
  uint16_t _last_us = 1500;
  bool     _attached = false;
};

} // namespace probot::devices

#else // !ESP32: no-op stub so host builds/tests can include probot.h

namespace probot::devices {
class Servo {
public:
  bool attach(uint8_t, uint16_t = 500, uint16_t = 2500) { return false; }
  void writeMicroseconds(uint16_t) {}
  void write(float) {}
  uint16_t readMicroseconds() const { return 1500; }
  bool attached() const { return false; }
  void detach() {}
};
} // namespace probot::devices

#endif
