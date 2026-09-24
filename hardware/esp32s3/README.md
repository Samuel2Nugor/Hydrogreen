# Hardware

## Status

Draft — subject to team review and approval.

## Purpose

This directory documents the physical hardware for the ESP32-S3 variant of the MicroHydros sensor node: bill of materials, wiring, pinout and bring-up procedure. Firmware build and flash instructions live in `firmware/esp32s3/README.md`.

## Overview

The sensor node is an ESP32-S3 DevKitC-1.1 with three sensors:

| Sensor | Interface | Purpose |
| ------ | --------- | ------- |
| SHT31 breakout | I2C @ 0x44 | Internal air temperature and humidity |
| DS18B20 (waterproof, probe A) | 1-Wire | Water or nutrient-solution temperature |
| DS18B20 (waterproof, probe B) | 1-Wire | External air temperature |

Both DS18B20 probes share one 1-Wire bus and are distinguished at boot by their factory-unique 64-bit ROM address. See `firmware/esp32s3/README.md` for the two-pass role-assignment procedure.

## Bill of materials

| Component | Qty | Notes |
| --------- | --- | ----- |
| ESP32-S3 DevKitC-1.1 | 1 | Espressif dev board with USB-C, built-in Wi-Fi/BT |
| SHT31 breakout | 1 | I2C, factory address `0x44` (some vendors ship `0x45` — verify) |
| DS18B20 waterproof probe | 2 | 1-Wire, stainless steel enclosure with cable |
| 4.7 kΩ resistor | 1 | 1-Wire pull-up (data ↔ 3V3) |
| Breadboard | 1 | Any full-size |
| Jumper wires | ~10 | Mixed male-to-male / male-to-female |
| USB-C cable | 1 | Data-capable (not charge-only) |

Optional for the final housing: heat-shrink over DS18B20 joints, an enclosure with cable glands, screw terminals for probe leads.

## Pinout

| ESP32-S3 pin | Connected to | Function |
| ------------ | ------------ | -------- |
| `3V3` | SHT31 `VIN`, both DS18B20 `VCC` (red), 4.7 kΩ (one leg) | 3.3 V power |
| `GND` | SHT31 `GND`, both DS18B20 `GND` (black) | Common ground |
| `GPIO4` | Both DS18B20 `DQ` (yellow/white), 4.7 kΩ (other leg → 3V3) | 1-Wire data bus |
| `GPIO8` | SHT31 `SDA` | I2C data |
| `GPIO9` | SHT31 `SCL` | I2C clock |

The 4.7 kΩ pull-up sits between the 1-Wire data line (`GPIO4`) and `3V3`. Without it the bus floats and no probe replies.

## Wiring diagram

```
                              ESP32-S3 DevKitC-1.1
                          ┌─────────────────────────┐
                     3V3 ─┤ 3V3                 GND ├─ GND
                          │                         │
             SHT31 SDA ───┤ GPIO8                   │
             SHT31 SCL ───┤ GPIO9                   │
                          │                         │
           1-Wire data ───┤ GPIO4                   │
                          └─────────────────────────┘
                                     │
              ┌──────────────────────┼──────────────────────┐
              │                      │                      │
        4.7 kΩ pull-up          DS18B20 water        DS18B20 external
        (to 3V3)                DQ, VCC, GND         DQ, VCC, GND

              ┌───────────────┐
    SHT31 →   │ VIN → 3V3     │
              │ GND → GND     │
              │ SDA → GPIO8   │
              │ SCL → GPIO9   │
              └───────────────┘
```

Both DS18B20 probes wire in parallel: three wires (`VCC`, `GND`, `DQ`) go to the same three breadboard rails.

## Power notes

- The DevKitC-1.1 delivers 3V3 from its onboard regulator when powered over USB. Do **not** feed 5 V into the sensor VCC pins.
- All grounds are common. A star ground at the breadboard rail is sufficient at this scale.
- USB power from a laptop is enough for prototype work. For deployment, use a 5 V USB power supply rated ≥500 mA.

## Sensor bring-up procedure

Follow this order the first time the hardware is assembled. Firmware commands live in `firmware/esp32s3/README.md`.

1. **Visual check** — with USB disconnected, verify every jumper against the pinout table. Confirm the pull-up resistor bridges `GPIO4` to `3V3`, not to `GND`.
2. **Power on and detect SHT31** — plug USB in, open the serial monitor at 115200 baud. Boot log should say:
   ```
   I sensor: SHT31 @0x44: detected
   ```
   If it says `NOT detected`: check SDA/SCL are not swapped and VIN is on 3V3, not 5 V.
3. **Detect DS18B20 probes** — the same boot log lists discovered ROMs:
   ```
   I sensor: DS18B20 discovered ROM: 28A1B2C3D4E5F601
   I sensor: DS18B20 discovered ROM: 28112233445566AA
   ```
   Two lines = both probes on bus. Zero or one line = missing pull-up, dead probe, or loose wire.
4. **Identify the water probe** — with the monitor open, dip one probe in warm water (~30 °C). Within one 30 s cycle its reading rises visibly; note which ROM belongs to which physical probe.
5. **Pin the ROMs** — set `CONFIG_DS18B20_WATER_ROM` and `CONFIG_DS18B20_EXTERNAL_ROM` in `idf.py menuconfig`, re-flash, and confirm the log reports `water=ready external=ready` without any `UNPINNED` warning.

## Physical placement (deployment)

- **Water probe** — into the reservoir. The stainless enclosure is waterproof; the cable joint is not. Keep the joint above the waterline.
- **External probe** — measures ambient air around the growing area. Mount out of direct sunlight and away from the water-probe cable so heat from one doesn't bias the other.
- **SHT31** — measures inside-enclosure conditions. Mount inside the electronics housing with airflow to the outside, otherwise it reports case temperature rather than room air.
- **1-Wire cable length** — keep under ~3 m for reliable operation with a single 4.7 kΩ pull-up. Longer runs may need a stronger pull-up or an active bus driver.

## Known constraints

- **DS18B20 conversion time** — ~750 ms at 12-bit resolution. Reading both probes sequentially takes ~1.6 s per telemetry cycle, which is why the firmware moves sensor reads off the timer callback (see `firmware/esp32s3/README.md` § Runtime behavior).
- **SHT31 address variant** — some vendor boards ship as `0x45` instead of `0x44`. If detection fails, verify with a bus scanner or check the vendor datasheet.
- **Probe swap after pinning** — if the two DS18B20 probes are physically swapped after ROMs are pinned in menuconfig, water and external readings will be reversed. Re-run bring-up step 4.

## Related documents

- `firmware/esp32s3/README.md` — build, flash, menuconfig options, expected serial output
- `docs/data-contract.md` — MQTT payload contract and plausibility ranges
- `docs/architecture.md` — full system diagram (sensors → MQTT → Node-RED → InfluxDB → Grafana)
