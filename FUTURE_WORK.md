# Future Work Notes

- **S-Curve Motion Profile Support**: Currently disabled due to high memory usage (up to 720KB worst-case for pre-computed trajectories). Future implementation options: (1) Sliding window approach (2.4KB per motor), (2) Analytical formulation (zero dynamic memory), or (3) Hybrid approach. Trapezoid profiles are available and sufficient for most use cases.
- Update NFR chassis geometry constants (track width, wheel base, wheel diameter) once official dimensions arrive.
- Integrate Boardoza motor controller support (PWM/CAN specifics pending from hardware team).
- Add hardware quadrature encoder driver implementation for ESP32-S3 (baseline example: PCNT + 1024 CPR wheel).
- Extend IMotorController to PIDF + feedforward slots and optional motion profile scheduling (trapezoid/S-curve).
- Swap placeholder NullMotorController usages with real IMotorController implementations in NFR examples.
- Expose chassis-level helpers to configure motor motion profiles and feedforward presets per drivetrain.
- Harden MPU6050 integration (calibration flow, failure handling) and prepare for future MPU9050/BNO variants.
- Validate joystick pipeline at 10 ms sampling + 20 ms control loop on hardware once robots are available.
- Add autonomous templates (10 cm forward → 90° turn → 10 cm forward) using finalized chassis parameters.
- Plan hardware-in-the-loop / field testing campaign when robots are ready.
- Implement battery voltage measurement using ESP32 ADC (voltage divider circuit) and expose via driver station UI.
- Add WiFi auto-reconnection mechanism for driver station when connection drops during competition.
- Review and optimize memory usage for motion profiles (SCurveProfile can allocate up to 720KB in worst case).
