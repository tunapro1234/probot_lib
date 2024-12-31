#ifndef probot_h
#define probot_h

// Fundamentals
#include "BaseClass/Robot.h"
#include "DriverStation/DriverStationNodeMCU.h"

// Raw Motor Controllers (without encoders)
#include "MotorController/BasicMotorController.h"
#include "MotorController/NFRRawMotor.h"

// Encoders
#include "Encoder/BasicEncoder.h"

// Controllers
#include "Controller/PIDController.h"
#include "Controller/PIDCoefficients.h"

// Feedback Motors (motors with encoders and pid control)
#include "BaseClass/CustomFeedbackMotor.h"
#include "MotorController/NFRFeedbackMotor.h"

// Chassis
#include "Chassis/BasicTankdrive.h"
#include "Chassis/FeedbackTankdrive.h"

// Sliders
#include "Slider/GenericSlider.h"
#include "Slider/NFRSlider.h"

#endif
