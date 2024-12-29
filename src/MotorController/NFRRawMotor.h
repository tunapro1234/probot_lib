#ifndef NFRRawMotor_h
#define NFRRawMotor_h

#include "../BaseClass/BaseMotorController.h"

class NFRRawMotor : public BaseMotorController {
public:
    NFRRawMotor(int id);

    void begin() override;
    void setPower(float power) override;
    void reset() override;

    int getID() const;

private:
    int motorID;
    float power;
};

#endif
