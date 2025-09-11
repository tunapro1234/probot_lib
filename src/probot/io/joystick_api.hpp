#ifndef PROBOT_IO_JOYSTICK_API_HPP
#define PROBOT_IO_JOYSTICK_API_HPP
#pragma once
#include <cmath>
#include <probot/io/gamepad.hpp>
#include <probot/io/joystick_mappings/mapping.hpp>

namespace probot::io::joystick_api {

struct Options {
  float deadzone = 0.08f;
  bool  invertY  = true;
};

class Joystick {
public:
  explicit Joystick(const IGamepadSource* source, Options opts = {})
  : _source(source), _opts(opts) {}

  // Low-level
  inline uint32_t getSeq() const { return snapshot().seq; }
  inline uint32_t getMs() const  { return snapshot().ms; }
  inline uint32_t getAxisCount() const { return snapshot().axisCount; }
  inline uint32_t getButtonCount() const { return snapshot().buttonCount; }
  inline bool isConnected() const { auto s=snapshot(); return (s.axisCount + s.buttonCount) > 0; }

  inline float getRawAxis(int i) const {
    auto s = snapshot();
    if (i < 0 || (uint32_t)i >= s.axisCount) return 0.0f;
    return s.axes[i];
  }

  inline bool getRawButton(int i) const {
    auto s = snapshot();
    if (i < 0 || (uint32_t)i >= s.buttonCount) return false;
    return s.buttons[i];
  }

  // WPILib-style POV: return -1 if none, else 0/90/180/270
  inline int getPOV() const {
    const auto& m = joystick_mapping::getActive();
    auto s = snapshot();
    if (m.dpadFromAxis && m.dpadAxis >= 0 && (uint32_t)m.dpadAxis < s.axisCount) {
      float v = s.axes[m.dpadAxis];
      if (v >= -1.0f && v < -0.5f) return 0;    // Up
      if (v >  0.0f && v <  0.5f)  return 90;   // Right
      if (v >  0.5f && v <= 1.0f)  return 180;  // Down
      if (v > -0.5f && v <  0.0f)  return 270;  // Left
      return -1;
    }
    // Try standard buttons 12..15 if available
    if (s.buttonCount > 15) {
      if (s.buttons[m.DPadUp])    return 0;
      if (s.buttons[m.DPadRight]) return 90;
      if (s.buttons[m.DPadDown])  return 180;
      if (s.buttons[m.DPadLeft])  return 270;
    } else if (s.buttonCount > 7) {
      // Legacy fallback 4..7
      if (s.buttons[4]) return 0;
      if (s.buttons[7]) return 90;
      if (s.buttons[5]) return 180;
      if (s.buttons[6]) return 270;
    }
    return -1;
  }

  // High-level axes using mapping
  inline float getLeftX()  const { return dz(getRawAxis(joystick_mapping::getActive().leftX)); }
  inline float getLeftY()  const { float y = getRawAxis(joystick_mapping::getActive().leftY); return dz(_opts.invertY ? -y : y); }
  inline float getRightX() const { return dz(getRawAxis(joystick_mapping::getActive().rightX)); }
  inline float getRightY() const { float y = getRawAxis(joystick_mapping::getActive().rightY); return dz(_opts.invertY ? -y : y); }

  inline float getLeftTriggerAxis()  const { return getRawButton(joystick_mapping::getActive().LT) ? 1.0f : 0.0f; }
  inline float getRightTriggerAxis() const { return getRawButton(joystick_mapping::getActive().RT) ? 1.0f : 0.0f; }

  // High-level buttons (Xbox names)
  inline bool getA() const { return getRawButton(joystick_mapping::getActive().A); }
  inline bool getB() const { return getRawButton(joystick_mapping::getActive().B); }
  inline bool getX() const { return getRawButton(joystick_mapping::getActive().X); }
  inline bool getY() const { return getRawButton(joystick_mapping::getActive().Y); }
  inline bool getLB() const { return getRawButton(joystick_mapping::getActive().LB); }
  inline bool getRB() const { return getRawButton(joystick_mapping::getActive().RB); }
  inline bool getBack() const { return getRawButton(joystick_mapping::getActive().Back); }
  inline bool getStart() const { return getRawButton(joystick_mapping::getActive().Start); }
  inline bool getOptions() const { return getRawButton(joystick_mapping::getActive().Options); }
  inline bool getLeftStickButton() const { return getRawButton(joystick_mapping::getActive().LStick); }
  inline bool getRightStickButton() const { return getRawButton(joystick_mapping::getActive().RStick); }

  // High-level buttons (PlayStation synonyms)
  inline bool getCross() const { return getA(); }
  inline bool getCircle() const { return getB(); }
  inline bool getSquare() const { return getX(); }
  inline bool getTriangle() const { return getY(); }

  // High-level D-Pad
  inline bool getDpadUp() const    { return getPOV() == 0; }
  inline bool getDpadRight() const { return getPOV() == 90; }
  inline bool getDpadDown() const  { return getPOV() == 180; }
  inline bool getDpadLeft() const  { return getPOV() == 270; }

private:
  inline float dz(float v) const { return (std::fabs(v) < _opts.deadzone) ? 0.0f : v; }
  inline GamepadSnapshot snapshot() const { return _source->read(); }

  const IGamepadSource* _source;
  Options _opts;
};

inline Joystick makeDefault(const Options& opts = {}) {
  return Joystick(&probot::io::gamepad(), opts);
}

} // namespace probot::io::joystick_api

#endif // PROBOT_IO_JOYSTICK_API_HPP 