set(PROBOT_SRCS
    src/core/runtime.cpp
    src/core/scheduler.cpp
    src/core/wdt.cpp
    src/io/ui_input.cpp
    src/platform/esp32s3/drivers/builtin_led_ws2812.cpp
    src/probot/devices/motors/boardoza_vnh5019_motor_driver.cpp
    src/probot/devices/motors/bts7960b_motor_driver.cpp
    src/probot/robot/system.cpp
    src/probot/sensors/imu/mpu6050.cpp
)

idf_component_register(
    SRCS ${PROBOT_SRCS}
    INCLUDE_DIRS "src"
    REQUIRES arduino
)

target_compile_definitions(${COMPONENT_TARGET}
    PUBLIC
        ARDUINO=10820
        ESP32
        ESP32S3
)
