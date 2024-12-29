#include "UpdateHelper.h"

UpdateHelper::UpdateHelper(unsigned long interval, unsigned long maxD)
  : updateInterval(interval), maxDelay(maxD), lastUpdateTime(0), lastOutput(0.0f), 
    loggingEnabled(false), hasLogged(false) {}

void UpdateHelper::setLogging(bool status) {
  loggingEnabled = status;
}

bool UpdateHelper::shouldUpdate() {
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentTime;
    hasLogged = false; // Reset log flag on successful update
    return true;
  }
  return false;
}

bool UpdateHelper::checkTimeout() {
  unsigned long currentTime = millis();
  if ((currentTime - lastUpdateTime) > maxDelay) {
    if (loggingEnabled && !hasLogged) {
      Serial.println(F("Warning: Update called too late!"));
      hasLogged = true; // Ensure only one log
      return true;
    }
  }
  return false;
}

void UpdateHelper::setLastOutput(float output) {
  lastOutput = output;
}

float UpdateHelper::getLastOutput() const {
  return lastOutput;
}

float UpdateHelper::getLastUpdateTime() const {
  return lastUpdateTime;
}

void UpdateHelper::setInterval(unsigned long interval) {
  updateInterval = interval;
}

void UpdateHelper::enableLogging(bool enable) {
  loggingEnabled = enable;
}
