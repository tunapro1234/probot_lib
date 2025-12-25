#include "test_harness.hpp"

#include <probot/io/gamepad.hpp>

extern unsigned long _test_millis_now;

TEST_CASE(gamepad_service_double_buffer){
  probot::io::GamepadService service;
  float axes[25];
  bool buttons[25];
  for (int i=0;i<25;i++){ axes[i] = static_cast<float>(i); buttons[i] = (i % 2) == 0; }

  service.write(100u, axes, 25u, buttons, 25u);
  _test_millis_now = 100;
  auto snap = service.read();
  _test_millis_now = 0;
  EXPECT_TRUE(snap.axisCount == 20u);
  EXPECT_TRUE(snap.buttonCount == 20u);
  EXPECT_NEAR(snap.axes[0], 0.0f, 1e-5f);
  EXPECT_TRUE(snap.buttons[0]);
}
