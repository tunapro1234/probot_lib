#ifndef UpdateHelper_h
#define UpdateHelper_h

#include <Arduino.h>

class UpdateHelper {
  public:
    // Constructor with default update interval (ms), max delay (ms), and logging flag
    UpdateHelper(unsigned long interval = 100, unsigned long maxDelay = 200);
    
    void setLogging(bool status);
    
    // Check if it's time to update
    bool shouldUpdate();
    
    // Check for timeout and log if necessary
    bool checkTimeout();
    
    // Set the last output value
    void setLastOutput(float output);
    
    // Get the cached last output
    float getLastOutput() const;
    
    float getLastUpdateTime() const;

    // Set a new update interval
    void setInterval(unsigned long interval);
    
    // Enable or disable logging
    void enableLogging(bool enable);
    
  private:
    unsigned long updateInterval;    // Time between updates
    unsigned long maxDelay;          // Maximum allowed delay before logging
    unsigned long lastUpdateTime;    // Last update timestamp
    float lastOutput;                // Cached output
    bool loggingEnabled;             // Logging flag
    bool hasLogged;                  // Flag to ensure single log
};

#endif
