#pragma once
#include <probot/controllers/pid.hpp>

namespace probot::nfr::detail {
  constexpr int kVelocitySlot = 0;
  constexpr int kPositionSlot = 1;

  constexpr probot::control::PidConfig kDefaultVelocityPid{0.2f, 0.0f, 0.0f, -1.0f, 1.0f};
  constexpr probot::control::PidConfig kDefaultPositionPid{0.6f, 0.0f, 0.02f, -1.0f, 1.0f};
}
