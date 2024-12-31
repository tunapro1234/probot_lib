#include "Robot.h"

Robot::Robot(unsigned long updateInterval, unsigned long maxDelay)
    : mode(RobotMode::DISABLED),
      autonomous(false),
      autoStartTime(0),
      autoPeriodLength(0),
      updateHelper(updateInterval, maxDelay) {}

void Robot::begin() {
    driverStation.begin();
    mode = RobotMode::DISABLED;
    autonomous = false;
    Serial.println("Robot initalized.");
}

unsigned long Robot::getAutoElapsed() const {
    return millis() - autoStartTime;
}

void Robot::updateStatus() {
    DSRobotStatus dsStatus = driverStation.getRobotStatus();
    enableAutonomous = driverStation.isAutonomousEnabled();
    autoPeriodLength = driverStation.getAutoPeriodLength() * 1000;
    
    if (dsStatus == DSRobotStatus::INIT) {
        autoStartTime = 0;
        mode = RobotMode::DISABLED;
    } 
    else if (dsStatus == DSRobotStatus::START) {
        mode = RobotMode::INITIALIZED;
    } 
    else if (dsStatus == DSRobotStatus::STOP) {
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
}

void Robot::update() {
    // robot güncelleme zamanı olmasa da clientları hallet
    driverStation.handleClient();

    // UpdateHelper ile güncelleme zamanı kontrolü
    if (!updateHelper.shouldUpdate()) {
        return; // Güncelleme zamanı değilse çık
    }

    updateStatus();

    // Zaman aşımı kontrolü
    // if (updateHelper.checkTimeout()) {
    //     Serial.println("Robot güncelleme zaman aşımı!");
    //     stop();
    //     return;
    // }

    // Güncelleme zamanını ve son çıktıyı kaydet
    // updateHelper.setLastOutput(static_cast<float>(mode));
}

void Robot::stop() {
    // driverStation.stop();
    mode = RobotMode::DISABLED;
    autonomous = false;
    Serial.println("Robot durduruldu.");
}

RobotMode Robot::getMode() const {
    return mode;
}

bool Robot::isAutonomous() const {
    return autonomous;
}
