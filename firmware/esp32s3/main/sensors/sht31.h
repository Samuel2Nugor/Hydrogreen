#pragma once

#include "sensor_types.h"

/* Sets up the I2C bus and probes for the SHT31 at 0x44. Safe to call even if
 * the sensor is absent — sht31_read() then reports "not_detected". */
void sht31_init(void);

/* Reads temperature (°C) and humidity (%RH) in one I2C transaction, verified
 * by CRC8. Both readings share one transaction, so both share one status. */
void sht31_read(sensor_reading_t *temp, sensor_reading_t *humidity);
