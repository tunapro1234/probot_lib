#ifndef ROBOT_H
#define ROBOT_H

#include "DriverStation.h"
#include "UpdateHelper.h"

// Enum for Robot Modes
enum class RobotMode {
    DISABLED,
    TELEOP,
    AUTONOMOUS
};

class Robot {
public:
    Robot(unsigned long updateInterval = 100, unsigned long maxDelay = 200);

    void begin(); // Başlatma
    void update(); // Güncelleme
    void stop(); // Durdurma

    void setMode(RobotMode mode);
    RobotMode getMode() const;

    bool isAutonomous() const;

private:
    DriverStation driverStation; // DriverStation nesnesi doğrudan burada oluşturuluyor
    RobotMode mode;               // Robot modu
    bool autonomous;              // Otonom mod durumu
    unsigned long autoStartTime;  // Otonom başlangıç zamanı
    unsigned long autoPeriodLength; // Otonom süre uzunluğu

    UpdateHelper updateHelper;    // Güncelleme yardımcısı
};

#endif // ROBOT_H
