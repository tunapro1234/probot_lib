#include "Robot.h"

Robot::Robot(unsigned long updateInterval, unsigned long maxDelay)
    : mode(RobotMode::DISABLED),
      autoStartTime(0),
      autoPeriodLength(0),
      updateHelper(updateInterval, maxDelay) {}

void Robot::begin() {
    driverStation.begin();
    mode = RobotMode::DISABLED;
    Serial.println("Robot initalized.");
}

unsigned long Robot::getAutoElapsed() const {
    return millis() - autoStartTime;
}

void Robot::updateStatus() {
    DSRobotStatus dsStatus = driverStation.getRobotStatus();
    enableAutonomous = driverStation.isAutonomousEnabled();
    autoPeriodLength = driverStation.getAutoPeriodLength() * 1000;
    
    // init tuşuna basılmışsa
    if (dsStatus == DSRobotStatus::INIT) {
        mode = RobotMode::INITIALIZED;
    } 

    // start tuşuna basılmışsa
    else if (dsStatus == DSRobotStatus::START) {
        // Otonom kapalıysa
        if (!enableAutonomous) {
            mode = RobotMode::TELEOP;
        } 
        // Otonom açıksa
        else {
            // Otonom modunda geçen süre 30 sn geçmediyse veya otonom daha başlamadıysa
            if (getAutoElapsed() < autoPeriodLength || autoStartTime == 0) {
                // Otonom modu aç
                if (autoStartTime == 0) {
                    // yeni başlıyosak süreyi kaydet
                    autoStartTime = millis();
                }
                mode = RobotMode::AUTONOMOUS;
            } 
            else {
                mode = RobotMode::TELEOP;
            }
        }
    } 

    // başlamamışsa ya da stop tuşuna basılmışsa
    else if (dsStatus == DSRobotStatus::STOP) {
        autoStartTime = 0;
        mode = RobotMode::DISABLED;
    }
}

void Robot::update() {
    // robot güncelleme zamanı olmasa da clientları hallet
    driverStation.handleClient();
    updateStatus();
}

void Robot::stop() {
    // driverStation.stop();
    mode = RobotMode::DISABLED;
    Serial.println("Robot durduruldu.");
}

RobotMode Robot::getMode() const {
    return mode;
}

bool Robot::isAutonomous() const {
    return mode == RobotMode::AUTONOMOUS;
}


const DriverStation* Robot::getDriverStation() const {
    return &driverStation;
}



bool Robot::isRobotInitialized() const {
    return m_isRobotInitialized;
}

bool Robot::isRobotEnded() const {
    return m_isRobotEnded;
}

bool Robot::isAutoInitialized() const {
    return m_isAutoInitialized;
}

bool Robot::isTeleopInitialized() const {
    return m_isTeleopInitialized;
}



void Robot::setRobotInitialized(bool status) {
    m_isRobotInitialized = status;
}

void Robot::setRobotEnded(bool status) {
    m_isRobotEnded = status;
}

void Robot::setAutoInitialized(bool status) {
    m_isAutoInitialized = status;
}

void Robot::setTeleopInitialized(bool status) {
    m_isTeleopInitialized = status;
}


// #ifdef ROBOT_FUNCTIONS
// bu kısım özel

Robot robot;

void setup() {
    Serial.begin(115200);
    robot.begin();

    Serial.println("Probot initialized!");
}

void loop() {
    robot.update();

    // Serial.print("Robot updated, mode: ");
    // Serial.print(static_cast<int>(robot.getMode()));
    // Serial.print(" Driver station mode: ");
    // Serial.println(static_cast<int>(robot.getDriverStation()->getRobotStatus()));

    if (robot.getMode() == RobotMode::DISABLED) {
        if (!robot.isRobotEnded()) {
            robotEnd();
            robot.setRobotEnded(true);
        }
        robot.setRobotInitialized(false);
        robot.setAutoInitialized(false);
        robot.setTeleopInitialized(false);
    }

    else if (robot.getMode() == RobotMode::INITIALIZED) {
        if (!robot.isRobotInitialized()) {
            robotInit();
            robot.setRobotInitialized(true);
        }
        robotInitLoop();
    }

    else if (robot.getMode() == RobotMode::AUTONOMOUS) {
        if (!robot.isAutoInitialized()) {
            autoInit();
            robot.setAutoInitialized(true);
        }
        // Otonom modda hareket et
        autoLoop();
    }

    else if (robot.getMode() == RobotMode::TELEOP) {
        if (!robot.isTeleopInitialized()) {
            teleopInit();
            robot.setTeleopInitialized(true);
        }
        // Teleop modda hareket et
        teleopLoop();
    }
}
// #endif
