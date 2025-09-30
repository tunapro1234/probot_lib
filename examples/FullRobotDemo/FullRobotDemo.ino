#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/sim/sim_motor.hpp>
#include <probot/sim/null_encoder.hpp>
#include <probot/devices/motors/motor_handle.hpp>

// Bu örnek, daha tamamlanmış bir robot iskeleti gösterir:
// - TankDrive şasi (teleop + otonom)
// - Intake (içeri alma) ve Shooter (fırlatma)
// - İki adet Slider ile tırmanma mekanizması (aç/kapa senaryosu)
// Not: NullMotor/SimEncoder yer tutucu (no-op) sürücülerdir.
// Gerçek projede bunları gerçek sürücülerle (örn. NFRMotor) değiştirin.
// Desteklenen motorlar için: https://docs.probotstudio.com/

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

// --- Dosya-üstü kurulum (sıralı, güvenli) ---
static const probot::control::PidConfig kPidCfg{ .kp=0.2f, .ki=0.0f, .kd=0.0f, .kf=0.0f, .out_min=-1.0f, .out_max=1.0f };
static probot::control::PID pidL(kPidCfg), pidR(kPidCfg);
static probot::sensors::SimEncoder leftEnc, rightEnc;   // yer tutucu
static probot::motor::NullMotor   leftHW, rightHW;       // yer tutucu
static probot::control::ClosedLoopMotor left(&leftEnc, &pidL, &leftHW, 1.0f, 1.0f);
static probot::control::ClosedLoopMotor right(&rightEnc, &pidR, &rightHW, 1.0f, 1.0f);
static probot::drive::BasicTankDrive chassis(&left, &right);

// Tırmanma sliderları (örnek amaçlı aynı motor/encoder ile)
static probot::mechanism::Slider sliderL(&left);
static probot::mechanism::Slider sliderR(&right);

// Intake/Shooter (no-op); gerçek projede gerçek motorla değiştirin
static probot::motor::NullMotor intakeHW;
static probot::motor::NullMotor shooterHW;
static probot::motor::MotorHandle intake(intakeHW);
static probot::motor::MotorHandle shooter(shooterHW);

// Tuş atamaları (UI tarafındaki buton indeksleri örnektir)
static const int BTN_INTAKE_IN   = 0; // A
static const int BTN_SHOOT       = 1; // B
static const int BTN_CLIMB_OPEN  = 8; // LB
static const int BTN_CLIMB_CLOSE = 9; // LT

// Otonom senaryo adımları
static uint32_t g_autoStep = 0;
static uint32_t g_autoMs   = 0;

void robotInit(){
  Serial.println("[FullRobot] robotInit: Başlatılıyor");
  // Örn: chassis.setWheelCircumference(31.4f); chassis.setTrackWidth(25.0f);
}

void robotEnd(){
  intake.setPower(0.0f);
  shooter.setPower(0.0f);
  Serial.println("[FullRobot] robotEnd: Bitti");
}

static void handleIntakeAndShooter(const probot::io::joystick_api::Joystick& js){
  bool intake_in  = js.getRawButton(BTN_INTAKE_IN);
  bool shoot_btn  = js.getRawButton(BTN_SHOOT);
  intake.setPower(intake_in ? 0.8f : 0.0f);
  shooter.setPower(shoot_btn ? 1.0f : 0.0f);
}

static void handleClimb(const probot::io::joystick_api::Joystick& js){
  bool open  = js.getRawButton(BTN_CLIMB_OPEN);
  bool close = js.getRawButton(BTN_CLIMB_CLOSE);

  if (open){
    sliderL.setTargetLength(40.0f); sliderR.setTargetLength(40.0f);
    uint32_t t0 = millis(); while (millis()-t0 < 2000){ sliderL.update(millis(), 20); sliderR.update(millis(), 20); delay(20);} 
    sliderL.setTargetLength(0.0f);  sliderR.setTargetLength(0.0f);
  }
  if (close){
    sliderL.setTargetLength(0.0f); sliderR.setTargetLength(0.0f);
  }

  sliderL.update(millis(), 20);
  sliderR.update(millis(), 20);
}

void teleopInit(){
  // Mapping değiştirmek için (varsayılan: "logitech-f310"):
  // probot::io::joystick_mapping::setActiveByName("standard");
  // probot::io::joystick_mapping::setActiveByName("logitech-f310");
  // probot::io::joystick_mapping::setActiveByName("axis9-dpad");
  Serial.println("[FullRobot] teleopInit: Tank sürüş + intake/shooter + climb");
}

void teleopLoop(){
  auto js = probot::io::joystick_api::makeDefault();
  float left_axis  = js.getLeftY();
  float right_axis = js.getRightY();
  float max_vel = 100.0f;
  chassis.setVelocity(left_axis*max_vel, right_axis*max_vel);
  chassis.update(millis(), 20);
  handleIntakeAndShooter(js);
  handleClimb(js);
  delay(20);
}

void autonomousInit(){
  Serial.println("[FullRobot] autonomousInit: Otonom başlayacak");
  g_autoStep = 0; g_autoMs = millis();
}

void autonomousLoop(){
  uint32_t now = millis();
  switch (g_autoStep){
    case 0:
      Serial.println("[FullRobot/Auto] 1) 50 cm ileri");
      chassis.driveDistance(50.0f);
      g_autoStep=1; g_autoMs=now; break;
    case 1:
      if (now - g_autoMs > 3000){
        Serial.println("[FullRobot/Auto] 2) Shooter çalıştır");
        shooter.setPower(1.0f);
        g_autoStep=2; g_autoMs=now;
      }
      break;
    case 2:
      if (now - g_autoMs > 2000){
        Serial.println("[FullRobot/Auto] 3) Shooter durdur");
        shooter.setPower(0.0f);
        g_autoStep=3; g_autoMs=now;
      }
      break;
    case 3:
      if (now - g_autoMs > 500){
        Serial.println("[FullRobot/Auto] 4) 30 cm ileri");
        chassis.driveDistance(30.0f);
        g_autoStep=4; g_autoMs=now;
      }
      break;
    default:
      break;
  }
  delay(20);
} 
