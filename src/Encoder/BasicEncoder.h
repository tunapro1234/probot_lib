#ifndef BasicEncoder_h
#define BasicEncoder_h

#include "../BaseClass/BaseEncoder.h"

class BasicEncoder : public BaseEncoder {
  public:
    BasicEncoder(int pinA, int pinB, bool isReversed = false);
    void begin() override;
    long getCount() override;
    void reset() override;

  private:
    bool isReversed;
    volatile long count;
    int pinA, pinB;
    static BasicEncoder* instance;

    static void handleInterrupt();
};

#endif
