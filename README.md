# DualSensorRCServoController
Dual-Sensor RC Servo Controller with Real-Time Angle Mapping (C, PIC32)

Author: Nikhil Thankasala

Overview:  
This project implements a real-time embedded system that uses either a rotary encoder or an ultrasonic distance sensor to control an RC servo motor through pulse-width modulation. The user can switch between input sources using a serial command, and the system continuously reports status and angle data back to a host interface over UART. It demonstrates integration of real-world sensor data, time-based processing, PWM signal generation, and SPI and input capture communication on a PIC32 microcontroller.

System Architecture:  
The system is composed of four key modules:

- **Free-Running Timer**: A high-precision timer using Timer5 provides global timebase access in both milliseconds and microseconds. This timer underpins periodic reporting and duration tracking across all other modules.

- **Rotary Encoder (SPI interface)**: A magnetic rotary encoder communicates over SPI in blocking mode to deliver 14-bit angle data. The system reads and scales this input to drive the servo position directly. The encoder is configured to return fresh position data at 100Hz.

- **Ultrasonic Distance Sensor (Ping)**: The HC-SR04 sensor is triggered periodically by a timer and uses input capture (Timer2 + IC3) to measure time-of-flight. A distance reading is derived from pulse width and then converted to a corresponding servo angle.

- **RC Servo Motor (PWM via Output Compare)**: The servo is controlled via a PWM signal generated using Output Compare 3 and Timer3. Pulse widths are updated based on sensor input, and values are clipped to ensure safe operating limits.

All modules are modularized and tested independently via conditional compilation and serial interface-based test harnesses. Communication with a host PC is done using a UART-based custom protocol, allowing dynamic sensor switching, pulse-width commands, and real-time debug output.

Implementation Strategy:  
The project began by designing a reusable timer library to track elapsed and absolute time. Next, the rotary encoder module was implemented using a blocking SPI protocol and validated using position reads at a fixed polling rate. The ultrasonic sensor was set up to run continuously using a trigger timer (Timer4) and an edge-detection ISR (Input Capture 3) for echo timing. Its readings were scaled linearly to simulate servo angle control based on object distance.

The servo PWM output was generated using Output Compare 3, with configurable pulse widths scaled from each sensor’s raw data. Finally, a unified application layer linked all subsystems and allowed seamless switching between encoder and ping inputs using serial commands. The system periodically reports current angle readings and status flags back to the host.

Outcome and Notes:  
The final application successfully maps rotary or ultrasonic input into smooth, real-time RC servo control. It allows toggling between absolute angle and derived distance modes, with visible PWM signal output verified by oscilloscope. Accurate timekeeping ensures consistent sensor timing and servo control. This project required careful coordination of timers, interrupts, scaling logic, and SPI/input capture configuration, providing a full-stack embedded systems design challenge.

Approximate development time: 18–20 hours. Potential next steps include support for closed-loop PID feedback, analog voltage inputs, or dynamic pulse-width smoothing for improved servo response.
