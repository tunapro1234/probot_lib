#include "test_harness.hpp"

#include <probot/core/lifecycle.hpp>
#include <probot/robot/state.hpp>

#include <string>
#include <vector>

namespace robot = probot::robot;
namespace core  = probot::core;
using core::Mode;

namespace {
  std::vector<std::string> g_calls;
  int g_transitions = 0;

  void h_robotInit()      { g_calls.push_back("robotInit"); }
  void h_robotEnd()       { g_calls.push_back("robotEnd"); }
  void h_teleopInit()     { g_calls.push_back("teleopInit"); }
  void h_teleopLoop()     { g_calls.push_back("teleopLoop"); }
  void h_autoInit()       { g_calls.push_back("autonomousInit"); }
  void h_autoLoop()       { g_calls.push_back("autonomousLoop"); }
  void h_onTransition()   { g_transitions++; }

  core::Hooks mkHooks(){
    return core::Hooks{ h_robotInit, h_robotEnd, h_teleopInit,
                        h_teleopLoop, h_autoInit, h_autoLoop };
  }
}

TEST_CASE(mode_for_status){
  EXPECT_TRUE(core::modeForStatus(robot::Status::STOP,  false) == Mode::STOP);
  EXPECT_TRUE(core::modeForStatus(robot::Status::INIT,  true)  == Mode::INIT);
  EXPECT_TRUE(core::modeForStatus(robot::Status::START, false) == Mode::TELEOP);
  EXPECT_TRUE(core::modeForStatus(robot::Status::START, true)  == Mode::AUTON);
}

TEST_CASE(lifecycle_full_transition_path){
  g_calls.clear(); g_transitions = 0;
  robot::StateService rs;
  core::PhaseMachine m;
  auto h = mkHooks();
  volatile uint32_t hb = 0;

  // STOP -> INIT: robotInit runs, phase becomes INITED, no loop hook.
  m.step(h, Mode::INIT, rs, 100, h_onTransition, &hb);
  EXPECT_TRUE(m.current() == Mode::INIT);
  EXPECT_TRUE(rs.read().phase == robot::Phase::INITED);
  EXPECT_TRUE(g_calls.size() == 1);
  EXPECT_TRUE(g_calls[0] == "robotInit");

  // INIT -> TELEOP: teleopInit then teleopLoop; heartbeat reset on entry.
  m.step(h, Mode::TELEOP, rs, 200, h_onTransition, &hb);
  EXPECT_TRUE(rs.read().phase == robot::Phase::TELEOP);
  EXPECT_TRUE(g_calls.size() == 3);
  EXPECT_TRUE(g_calls[1] == "teleopInit");
  EXPECT_TRUE(g_calls[2] == "teleopLoop");
  EXPECT_TRUE(hb == 200u);

  // Stay in TELEOP: just another loop, no transition.
  m.step(h, Mode::TELEOP, rs, 220, h_onTransition, &hb);
  EXPECT_TRUE(g_calls.size() == 4);
  EXPECT_TRUE(g_calls[3] == "teleopLoop");

  // TELEOP -> STOP: robotEnd, phase NOT_INIT.
  m.step(h, Mode::STOP, rs, 300, h_onTransition, &hb);
  EXPECT_TRUE(rs.read().phase == robot::Phase::NOT_INIT);
  EXPECT_TRUE(g_calls.back() == "robotEnd");

  // Three real transitions (INIT, TELEOP, STOP), not the stay-in-TELEOP.
  EXPECT_TRUE(g_transitions == 3);
}

TEST_CASE(lifecycle_autonomous_entry_stamps_start){
  g_calls.clear();
  robot::StateService rs;
  core::PhaseMachine m;
  auto h = mkHooks();

  m.step(h, Mode::AUTON, rs, 500, nullptr, nullptr);
  auto s = rs.read();
  EXPECT_TRUE(s.phase == robot::Phase::AUTONOMOUS);
  EXPECT_TRUE(s.autoStartMs == 500u);
  EXPECT_TRUE(g_calls.size() == 2);
  EXPECT_TRUE(g_calls[0] == "autonomousInit");
  EXPECT_TRUE(g_calls[1] == "autonomousLoop");
}

TEST_CASE(supervisor_modes_and_estop){
  robot::StateService rs;
  core::Supervisor sup;

  rs.setStatus(0, robot::Status::STOP);
  EXPECT_TRUE(sup.update(rs, 10, false) == Mode::STOP);

  rs.setStatus(0, robot::Status::INIT);
  EXPECT_TRUE(sup.update(rs, 10, false) == Mode::INIT);

  rs.setStatus(0, robot::Status::START);
  rs.setAutonomous(0, false);
  EXPECT_TRUE(sup.update(rs, 10, false) == Mode::TELEOP);

  rs.setAutonomous(0, true);
  EXPECT_TRUE(sup.update(rs, 10, false) == Mode::AUTON);

  // Latched estop overrides everything.
  EXPECT_TRUE(sup.update(rs, 10, true) == Mode::STOP);
}

TEST_CASE(supervisor_auto_period_expiry){
  robot::StateService rs;
  core::Supervisor sup;
  rs.setStatus(0, robot::Status::START);
  rs.setAutonomous(0, true);
  rs.setPhase(0, robot::Phase::AUTONOMOUS);   // user task has entered auton
  rs.setAutoStartMs(0, 1000);
  rs.setAutoPeriodSeconds(0, 5);              // 5 s window

  // 4 s in: still autonomous.
  EXPECT_TRUE(sup.update(rs, 5000, false) == Mode::AUTON);
  EXPECT_TRUE(rs.read().autonomousEnabled == true);

  // 5 s in: period elapsed -> autonomous flag dropped -> TELEOP.
  EXPECT_TRUE(sup.update(rs, 6000, false) == Mode::TELEOP);
  EXPECT_TRUE(rs.read().autonomousEnabled == false);
}

TEST_CASE(stall_detection){
  using core::isStalled;
  EXPECT_TRUE(isStalled(robot::Phase::TELEOP, 5000, 1000, 2000) == true);   // 4000 > 2000
  EXPECT_TRUE(isStalled(robot::Phase::TELEOP, 2500, 1000, 2000) == false);  // 1500 < 2000
  EXPECT_TRUE(isStalled(robot::Phase::INITED, 5000, 1000, 2000) == false);  // not a loop phase
  EXPECT_TRUE(isStalled(robot::Phase::AUTONOMOUS, 5000, 0, 2000) == false); // no loop yet (hb==0)
}
