#include "BasicEncoder.h"

BasicEncoder* BasicEncoder::instance = nullptr;

BasicEncoder::BasicEncoder(int a, int b) : pinA(a), pinB(b), count(0) {
  instance = this;
}

void BasicEncoder::begin() {
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinA), handleInterrupt, CHANGE);
}

long BasicEncoder::getCount() {
  noInterrupts();
  long currentCount = count;
  interrupts();
  return currentCount;
}

void BasicEncoder::reset() {
  noInterrupts();
  count = 0;
  interrupts();
}

void BasicEncoder::handleInterrupt() {
  if (instance) {
    if (digitalRead(instance->pinA) == digitalRead(instance->pinB)) {
      instance->count++;
    } else {
      instance->count--;
    }
  }
}
