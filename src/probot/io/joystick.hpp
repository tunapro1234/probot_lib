#ifndef PROBOT_JOYSTICK_HPP
#define PROBOT_JOYSTICK_HPP
#pragma once
#include <stdint.h>

namespace probot { struct InputState; InputState read_input_snapshot(); }

namespace joystick {
  struct State {
    int16_t  x;
    int16_t  y;
    bool     btn;
    uint32_t ms;
    uint32_t seq;
  };

  inline State read(){
    probot::InputState s = probot::read_input_snapshot();
    return { s.x, s.y, (bool)(s.btn != 0), s.ms, s.seq };
  }

  inline int16_t  get_axisX(){ return probot::read_input_snapshot().x; }
  inline int16_t  get_axisY(){ return probot::read_input_snapshot().y; }
  inline bool     get_button(){ return probot::read_input_snapshot().btn != 0; }
  inline uint32_t get_ms(){ return probot::read_input_snapshot().ms; }
}
#endif // PROBOT_JOYSTICK_HPP
