#include "test_harness.hpp"

#include <probot/io/gamepad.hpp>

TEST_CASE(gamepad_service_double_buffer){
  probot::io::GamepadService service;
  float axes[25];
  bool buttons[25];
  for (int i=0;i<25;i++){ axes[i] = static_cast<float>(i); buttons[i] = (i % 2) == 0; }

  service.write(100u, axes, 25u, buttons, 25u);
  auto snap = service.read();
  EXPECT_TRUE(snap.axisCount == 20u);
  EXPECT_TRUE(snap.buttonCount == 20u);
  EXPECT_NEAR(snap.axes[0], 0.0f, 1e-5f);
  EXPECT_TRUE(snap.buttons[0]);
}
