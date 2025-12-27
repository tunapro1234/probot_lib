idf_component_register(
    INCLUDE_DIRS "src"
    REQUIRES arduino
)

target_compile_definitions(${COMPONENT_TARGET}
    PUBLIC
        ARDUINO=10820
        ESP32
        ESP32S3
)
