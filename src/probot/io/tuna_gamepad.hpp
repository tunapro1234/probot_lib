#ifndef PROBOT_IO_TUNA_GAMEPAD_HPP
#define PROBOT_IO_TUNA_GAMEPAD_HPP
#pragma once
#include <stdint.h>
#include <probot/io/gamepad.hpp>

namespace probot::io {
  enum class Axis : int { LeftJoystickX=0, LeftJoystickY=1, RightJoystickX=2, RightJoystickY=3, LT=4, RT=5 };
  enum class Button : int { A=0, B=1, X=2, Y=3, DPadUp=4, DPadDown=5, DPadLeft=6, DPadRight=7, LB=8, LT=9, RB=10, RT=11, Options=12, Start=13 };

  class TunaGamepad {
  public:
    explicit TunaGamepad(const IGamepadSource* src) : _src(src){ for(int i=0;i<6;i++) _axisOffsets[i]=0.0f; }

    void calibrate(){ auto s = _src->read(); for (int i=0;i<4;i++){ int m = axisMapping[i]; if (m>=0 && (uint32_t)m < s.axisCount) _axisOffsets[i] = s.axes[m]; } }

    float getAxis(Axis axis) const {
      auto s = _src->read(); int logical = (int)axis;
      if (logical>=0 && logical<4){ int m=axisMapping[logical]; if (m>=0 && (uint32_t)m<s.axisCount) return s.axes[m] - _axisOffsets[logical]; return 0.0f; }
      if (logical==4){ // LT as button
        if ((uint32_t)7 < s.buttonCount) return s.buttons[7] ? 1.0f : 0.0f; return 0.0f;
      }
      if (logical==5){ // RT as button
        if ((uint32_t)6 < s.buttonCount) return s.buttons[6] ? 1.0f : 0.0f; return 0.0f;
      }
      return 0.0f;
    }

    bool getButton(Button button) const {
      auto s = _src->read(); int logical = (int)button;
      if (logical>=4 && logical<=7){ // DPad from axis9
        if (s.axisCount>9){ float v = s.axes[9];
          if (logical==4 && v>=-1.0f && v<-0.5f) return true;
          if (logical==5 && v>0.0f  && v< 0.5f) return true;
          if (logical==6 && v>0.5f  && v<=1.0f) return true;
          if (logical==7 && v>-0.5f && v< 0.0f) return true;
        }
        return false;
      }
      if (logical>=0 && logical<14){ int m = buttonMapping[logical]; if (m>=0 && (uint32_t)m < s.buttonCount) return s.buttons[m]; }
      return false;
    }

  private:
    static constexpr int axisMapping[6] = { 0, 1, 2, 5, -1, -1 };
    static constexpr int buttonMapping[14] = { 2,1,3,0, -1,-1,-1,-1, 4,5,8,9, 10,11 };

    const IGamepadSource* _src;
    float _axisOffsets[6];
  };
}
#endif // PROBOT_IO_TUNA_GAMEPAD_HPP 