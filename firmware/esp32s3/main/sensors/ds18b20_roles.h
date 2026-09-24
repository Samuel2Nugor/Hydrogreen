#pragma once

#include "sensor_types.h"

#define DS18B20_MAX           2
#define DS18B20_ROLE_WATER    0
#define DS18B20_ROLE_EXTERNAL 1

/* Discovers every probe on the 1-Wire bus and assigns each to a role (water /
 * external) by pinned ROM address (menuconfig), falling back to discovery
 * order with a loud warning for anything unpinned or missing. */
void ds18b20_init_sensors(void);

/* Triggers a conversion and reads back the temperature for the given role.
 * Blocks ~800 ms (12-bit conversion time) — call from a task, not a timer
 * callback. */
void ds18b20_read_sensor(int role, sensor_reading_t *result);
