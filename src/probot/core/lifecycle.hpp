#pragma once
#include <stdint.h>
#include <probot/robot/state.hpp>

// Cooperative lifecycle core — pure logic, no FreeRTOS / Arduino deps so it
// builds and unit-tests on the host. The ESP32 plumbing (the persistent
// userTask, the sysloop, watchdogs, emergency stop) lives in runtime.hpp and
// drives these primitives.
//
// Design: there is exactly ONE long-lived user task. It never gets killed
// during normal operation — a kill mid-transaction is what orphaned a lock
// (Wire/malloc) and froze the robot in 0.2.x. Phase changes are cooperative:
// the supervisor publishes a "requested mode"; the user task observes it at a
// loop boundary and runs the transition hooks itself. The only force is a
// full reboot (watchdog) or the terminal emergency stop.

namespace probot::core {

  enum class Mode : uint8_t { STOP = 0, INIT = 1, TELEOP = 2, AUTON = 3 };

  // User lifecycle hooks, indirected so the phase machine is testable with
  // mock callbacks. Any may be null (treated as empty).
  struct Hooks {
    void (*robotInit)();
    void (*robotEnd)();
    void (*teleopInit)();
    void (*teleopLoop)();
    void (*autonomousInit)();
    void (*autonomousLoop)();
  };

  // Which mode the user task should be in for a given button Status. Pure.
  inline Mode modeForStatus(robot::Status st, bool autonomousEnabled) {
    switch (st) {
      case robot::Status::INIT:  return Mode::INIT;
      case robot::Status::START: return autonomousEnabled ? Mode::AUTON : Mode::TELEOP;
      case robot::Status::STOP:
      default:                   return Mode::STOP;
    }
  }

  inline robot::Phase phaseForMode(Mode m) {
    switch (m) {
      case Mode::INIT:   return robot::Phase::INITED;
      case Mode::TELEOP: return robot::Phase::TELEOP;
      case Mode::AUTON:  return robot::Phase::AUTONOMOUS;
      case Mode::STOP:
      default:           return robot::Phase::NOT_INIT;
    }
  }

  // Runs inside the single persistent user task. Owns the current mode and
  // drives hook calls at safe loop boundaries — never mid-hook, so a stop or
  // phase change can never kill user code holding a lock.
  class PhaseMachine {
  public:
    Mode current() const { return cur_; }

    // One iteration. If the requested mode changed, run the transition
    // (zero inputs, then the enter hook for the new mode, publishing the
    // public Phase). Then call the active loop hook once. `onTransition`
    // (e.g. zero the joystick) runs once per transition; `hb`, if given, is
    // refreshed on entry to a loop phase so the stall watchdog starts clean.
    void step(const Hooks& h, Mode requested, robot::StateService& rs, uint32_t now,
              void (*onTransition)() = nullptr, volatile uint32_t* hb = nullptr) {
      if (requested != cur_) {
        if (onTransition) onTransition();
        enter(h, requested, rs, now);
        cur_ = requested;
        if (hb && (cur_ == Mode::TELEOP || cur_ == Mode::AUTON)) *hb = now;
      }
      if (cur_ == Mode::TELEOP) { if (h.teleopLoop) h.teleopLoop(); }
      else if (cur_ == Mode::AUTON) { if (h.autonomousLoop) h.autonomousLoop(); }
    }

  private:
    void enter(const Hooks& h, Mode m, robot::StateService& rs, uint32_t now) {
      switch (m) {
        case Mode::STOP:
          if (h.robotEnd) h.robotEnd();
          break;
        case Mode::INIT:
          if (h.robotInit) h.robotInit();
          break;
        case Mode::TELEOP:
          if (h.teleopInit) h.teleopInit();
          break;
        case Mode::AUTON:
          // Stamp the autonomous start BEFORE the init hook so the period
          // timer (and the UI countdown) reference this run, not a stale one.
          rs.setAutoStartMs(now, now);
          if (h.autonomousInit) h.autonomousInit();
          break;
      }
      rs.setPhase(now, phaseForMode(m));
    }

    Mode cur_ = Mode::STOP;
  };

  // Runs inside the sysloop (supervisor). Translates button Status into the
  // requested mode and enforces the autonomous period — cooperatively, by
  // flipping the autonomous flag so the mode resolves to TELEOP (no task
  // kill). Stateless apart from the StateService it reads/writes.
  class Supervisor {
  public:
    // Returns the mode the user task should run. When estop is latched the
    // answer is always STOP.
    Mode update(robot::StateService& rs, uint32_t now, bool estopLatched) {
      if (estopLatched) return Mode::STOP;
      auto s = rs.read();
      // Autonomous period expiry. Gate on phase==AUTONOMOUS so we only count
      // once the user task has actually entered autonomous and stamped a
      // fresh autoStartMs — otherwise a stale timestamp from a previous run
      // would expire the new run instantly.
      if (s.status == robot::Status::START && s.autonomousEnabled &&
          s.phase == robot::Phase::AUTONOMOUS && s.autoStartMs != 0 &&
          s.autoPeriodSeconds > 0 &&
          (int32_t)(now - s.autoStartMs) >= s.autoPeriodSeconds * 1000) {
        rs.setAutonomous(now, false);
        s.autonomousEnabled = false;
      }
      return modeForStatus(s.status, s.autonomousEnabled);
    }
  };

  // Stall detection: a loop iteration that hasn't returned within the
  // deadline. Pure so it is unit-testable.
  inline bool isStalled(robot::Phase phase, uint32_t now, uint32_t heartbeat_ms,
                        uint32_t deadline_ms) {
    bool loopPhase = (phase == robot::Phase::TELEOP || phase == robot::Phase::AUTONOMOUS);
    return loopPhase && heartbeat_ms != 0 &&
           (int32_t)(now - heartbeat_ms) > (int32_t)deadline_ms;
  }

} // namespace probot::core
