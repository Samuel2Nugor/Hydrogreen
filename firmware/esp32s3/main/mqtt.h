#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sensor_types.h"

/* Starts the MQTT client with LWT ("offline") configured before connect;
 * publishes retained "online" status once connected. boot_id is copied. */
void mqtt_init(const char *boot_id);

/* Blocks up to timeout_ms (in 500 ms steps) for the initial connection, then
 * returns regardless — the client keeps retrying in the background. Logs a
 * warning if it timed out. Returns whether it is connected. */
bool mqtt_wait_connected(uint32_t timeout_ms);

/* Builds and publishes one telemetry JSON message per the v1 data contract.
 * No-op (with a warning) if not currently connected. */
void mqtt_publish_telemetry(const sensor_reading_t *internal_temp,
                            const sensor_reading_t *internal_hum,
                            const sensor_reading_t *water_temp,
                            const sensor_reading_t *external_temp);
