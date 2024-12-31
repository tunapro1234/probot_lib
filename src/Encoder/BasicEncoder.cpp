#include "BasicEncoder.h"

BasicEncoder* BasicEncoder::instance = nullptr;

BasicEncoder::BasicEncoder(int a, int b, bool isReversed) : pinA(a), pinB(b), isReversed(isReversed), count(0) {
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
  return isReversed ? -currentCount : currentCount;
}

void BasicEncoder::reset() {
  noInterrupts();
  count = 0;
  isReversed = false;
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
