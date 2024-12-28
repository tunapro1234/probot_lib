#ifndef PIDCoefficients_h
#define PIDCoefficients_h

class PIDCoefficients {
public:
    // Constructor
    PIDCoefficients(float kp = 0.0, float ki = 0.0, float kd = 0.0);

    // Getters
    float getKp() const;
    float getKi() const;
    float getKd() const;

    // Setters
    void setKp(float kp);
    void setKi(float ki);
    void setKd(float kd);

    // Operator Overloading (for convenience)
    void operator=(const PIDCoefficients& other);

private:
    float kp; // Proportional gain
    float ki; // Integral gain
    float kd; // Derivative gain
};

#endif
