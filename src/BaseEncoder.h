#ifndef BaseEncoder_h
#define BaseEncoder_h

#include <Arduino.h>

class BaseEncoder {
  public:
    virtual void begin() = 0;
    virtual long getCount() = 0;
    virtual void reset() = 0;
};

#endif