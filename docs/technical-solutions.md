# Technical Solution

> **Status:** Planned. This document will be developed and reviewed by the team.

## Hardware

### Development board

- **ESP32-S3**
- Built-in Wi-Fi and good MQTT support.

### Sensors

- **SHT31** for internal humidity and temperature
	- Good accuracy and stability.
	- Works well with the Raspberry Pi Pico and ESP32.
	- Uses I2C.
- **DS18B20** for water temperature
	- Accuracy of approximately +/- 0.5 C.
	- Works well with the Raspberry Pi Pico and ESP32.
	- Widely supported by software libraries.

## Software

### Firmware

- Arduino/C++ or MicroPython.
- Reads the sensors, validates the measurements, and sends the data.

### Communication

- MQTT over Wi-Fi.
- Lightweight communication supported by both the ESP32-S3 and Raspberry Pi Pico 2 W.

### MQTT broker

- **Mosquitto**
- Receives measurements published by the development board.

### Backend and data storage

- **Node-RED with InfluxDB**
- Node-RED processes MQTT messages, while InfluxDB stores historical sensor data.

### Version control

- **GitHub**
- Stores the code and documentation and allows the whole team to contribute.

## Communication Details

### Sensors to development board

- The SHT31 uses I2C, while the DS18B20 uses 1-Wire.

### Communication with external systems

- Wi-Fi provides the network connection because both the ESP32-S3 and Raspberry Pi Pico 2 W have built-in Wi-Fi.

### Data protocol

- MQTT sends measurements to an MQTT broker.
- It is lightweight, suitable for IoT, and can later support multiple MicroHydros units.

### Security

- If time permits, authentication and encrypted MQTT communication using TLS will be implemented.
