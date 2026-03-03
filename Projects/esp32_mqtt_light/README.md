# ESP32 MQTT Light Sensor

This project reads an analog light sensor and publishes the value to an MQTT broker if it exceeds a threshold. The threshold can be adjusted remotely via MQTT.

## Hardware Setup
- **ESP32 Board**
- **Light Sensor (LDR)**:
  - VCC to 3.3V
  - GND to GND
  - Signal to **GPIO 34** (Analog Input)
  - *Note*: You may need a voltage divider (e.g., 10k resistor) depending on your LDR module.

## Software Setup
1. Open this folder in **VS Code** with the **PlatformIO** extension installed.
2. Open `src/main.cpp`.
3. Update the following constants at the top of the file:
   - `ssid`: Your WiFi Name
   - `password`: Your WiFi Password
   - `mqtt_server`: IP address of your MQTT Broker

## MQTT Topics
- `sensors/light/value` (Publish): The light sensor reading (only sent if > threshold).
- `sensors/light/threshold/set` (Subscribe): Send an integer payload to this topic to update the threshold (e.g., "3000").
- `sensors/light/status` (Publish): Status messages and confirmation of threshold updates.

## Building and Flashing
1. Connect ESP32 via USB.
2. Click the PlatformIO **Upload** button (arrow icon).
3. Open the **Serial Monitor** (plug icon) to see debug output (Baud: 115200).
