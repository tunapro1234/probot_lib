#ifndef ROBOT_H
#define ROBOT_H

#include "../DriverStation/DriverStationNodeMCU.h"
#include "../Util/UpdateHelper.h"


// Enum for Robot Modes
enum class RobotMode {
    DISABLED,
    INITIALIZED,
    TELEOP,
    AUTONOMOUS
};


class Robot {
public:
    Robot(unsigned long updateInterval = 100, unsigned long maxDelay = 200);

    void begin();
    void update();
    void stop();

    RobotMode getMode() const;
    const DriverStation* getDriverStation() const;
    unsigned long getAutoElapsed() const;

    bool isAutonomous() const;

    bool isRobotInitialized() const;
    bool isRobotEnded() const;
    bool isAutoInitialized() const;
    bool isTeleopInitialized() const;

    void setRobotInitialized(bool status);
    void setRobotEnded(bool status);
    void setAutoInitialized(bool status);
    void setTeleopInitialized(bool status);

private:
    void updateStatus();

    bool m_isRobotEnded;
    bool m_isRobotInitialized;
    bool m_isAutoInitialized;
    bool m_isTeleopInitialized;

    DriverStation driverStation; // DriverStation nesnesi doğrudan burada oluşturuluyor
    RobotMode mode;               // Robot modu
    bool enableAutonomous;        // Otonom modu açılabilir mi
    unsigned long autoStartTime;  // Otonom başlangıç zamanı
    unsigned long autoPeriodLength; // Otonom süre uzunluğu

    UpdateHelper updateHelper;    // Güncelleme yardımcısı
};



// #ifdef ROBOT_FUNCTIONS
// this part is special
void setup();
void loop();

void robotInit();
void robotInitLoop();
void robotEnd();
void autoInit();
void autoLoop();
void teleopInit();
void teleopLoop();

extern Robot robot;
// #endif // ROBOT_FUNCTIONS

#endif // ROBOT_H



