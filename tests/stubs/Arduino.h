#pragma once
#include <stdint.h>
#include <math.h>

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2

#ifdef __cplusplus
extern "C" {
#endif
void yield(void);
#ifdef __cplusplus
}
#endif

inline void yield(void) {}

extern unsigned long _test_millis_now;

inline unsigned long millis(){ return _test_millis_now; }
inline void delay(unsigned long ms){ _test_millis_now += ms; }
inline void delayMicroseconds(unsigned int us){ _test_millis_now += (us + 999u) / 1000u; }

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline int digitalRead(int){ return 0; }
inline void analogWrite(int, int) {}

struct __SerialStub {
  void begin(unsigned long) {}
  void println(const char*) {}
  void println(int) {}
  void print(const char*) {}
  void printf(const char*, ...) {}
} __attribute__((unused));

static __SerialStub Serial;

inline uint32_t micros(){ return millis() * 1000u; }

