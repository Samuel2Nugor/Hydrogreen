# Technical Solution

> **Status:** Planned. This document will be developed and reviewed by the team.

## Hardware

### Development board

- **ESP32-S3**
	- Built-in Wi-Fi (802.11 b/g/n), no separate radio module needed.
	- Native USB on the DevKitC-1.1 board, so no separate USB-serial chip to fail or drop drivers.
	- Dual-core with enough RAM/flash to run FreeRTOS with the MQTT client, JSON encoding and sensor drivers as separate tasks (see `firmware/esp32s3/README.md` § Runtime behavior) — more headroom than a single-core microcontroller would give for the same job.
	- Mature ESP-IDF tooling and documentation, and the team already had working drivers for I2C and 1-Wire on this platform.
	- Considered against the Raspberry Pi Pico 2 W (also Wi-Fi capable, cheaper, single-core RP2350): kept as a second, in-progress firmware target (see the open PR for the Pico build) rather than the primary board, since the ESP32-S3 path was further along and its native USB is more convenient for development.

### Sensors

DHT11 and DHT22 are excluded from consideration — the project explicitly prohibits them (see `docs/requirements.md` § CR-01).

- **SHT31** for internal air temperature and humidity
	- I2C, factory-calibrated, CRC-checked readings (see `firmware/esp32s3/main/sensors/sht31.c`).
	- Rated accuracy ≈ ±0.3 °C / ±2 %RH, range −40…125 °C / 0…100 %RH — well inside the growing-environment range this project needs.
	- Considered against **BME280** (also I2C, cheaper, but only measures temperature/humidity/pressure with looser typical humidity accuracy, ≈ ±3 %RH, and known long-term humidity-sensor drift) and **SHT4x** (similar accuracy to the SHT31, but no working driver or hardware on hand when sensor selection was made). The SHT31 was chosen for its combination of accuracy and an already-working driver.
- **DS18B20** (×2) for water/nutrient-solution and external air temperature
	- 1-Wire, accuracy ≈ ±0.5 °C over −10…+85 °C, 9–12-bit configurable resolution.
	- Each unit has a unique factory-programmed 64-bit ROM address, so both probes share one GPIO pin and one 1-Wire bus instead of needing two I2C addresses or a second bus — see `docs/architecture.md` § Sensor connections.
	- A waterproof probe variant is required for the water/nutrient-solution measurement in any case; using the same sensor family for the external-air measurement keeps the bill of materials and firmware driver to one shared implementation (see `firmware/esp32s3/main/sensors/ds18b20_roles.c`) instead of adding a second I2C sensor and its own address-conflict handling.
	- Widely supported by software libraries, including the Espressif `onewire_bus`/`ds18b20` components already used by this firmware.

## Software

### Firmware

- ESP-IDF (C) on the ESP32-S3 — see `firmware/esp32s3/`.
- Reads the sensors, checks read status per sensor, and publishes raw telemetry. Plausibility validation happens in Node-RED, not on-device (see `docs/architecture.md` § Validation and failure handling).

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
