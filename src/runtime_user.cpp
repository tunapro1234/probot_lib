#include <Arduino.h>
#include <probot.h>

// Declarations for user-provided robot lifecycle hooks (defined in the sketch)
void robotInit();
void robotEnd();
void teleopInit();
void teleopLoop();
void autonomousInit();
void autonomousLoop();

// Arduino entry points
void setup(){
  probot::runtime_setup();
}

void loop(){
  delay(100);
}

// User task scheduled on control core from probot::runtime_setup()
namespace probot {
void userTask(void*){
  robotInit();
  teleopInit();

  for(;;){
    teleopLoop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  robotEnd();
}
} // namespace probot 