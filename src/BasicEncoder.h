#ifndef BasicEncoder_h
#define BasicEncoder_h

#include "BaseEncoder.h"

class BasicEncoder : public BaseEncoder {
  public:
    BasicEncoder(int pinA, int pinB);
    void begin() override;
    long getCount() override;
    void reset() override;

  private:
    volatile long count;
    int pinA, pinB;
    static BasicEncoder* instance;

    static void handleInterrupt();
};

#endif
