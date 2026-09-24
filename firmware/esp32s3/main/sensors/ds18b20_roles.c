/*
 * MicroHydros firmware — DS18B20 (1-Wire) role assignment and reads.
 *
 * Both probes share one 1-Wire bus; role (water / external) is pinned by ROM
 * address via menuconfig, with a warned fallback to discovery order.
 */
#include "ds18b20_roles.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "onewire_bus.h"
#include "ds18b20.h"

#define ONEWIRE_GPIO CONFIG_ONEWIRE_GPIO

static const char *TAG_SENSOR = "sensor";

static onewire_bus_handle_t    s_ow_bus = NULL;
static ds18b20_device_handle_t s_ds18b20[DS18B20_MAX];       /* indexed by ROLE */
static bool                    s_ds18b20_present[DS18B20_MAX];

void ds18b20_init_sensors(void)
{
    for (int i = 0; i < DS18B20_MAX; i++) {
        s_ds18b20[i] = NULL;
        s_ds18b20_present[i] = false;
    }

    const uint64_t role_rom[DS18B20_MAX] = {
        [DS18B20_ROLE_WATER]    = strtoull(CONFIG_DS18B20_WATER_ROM,    NULL, 16),
        [DS18B20_ROLE_EXTERNAL] = strtoull(CONFIG_DS18B20_EXTERNAL_ROM, NULL, 16),
    };

    onewire_bus_config_t bus_cfg = { .bus_gpio_num = ONEWIRE_GPIO };
    onewire_bus_rmt_config_t rmt_cfg = { .max_rx_bytes = 10 };
    if (onewire_new_bus_rmt(&bus_cfg, &rmt_cfg, &s_ow_bus) != ESP_OK) {
        ESP_LOGE(TAG_SENSOR, "1-Wire bus init failed — skipping DS18B20");
        return;
    }

    onewire_device_iter_handle_t iter;
    if (onewire_new_device_iter(s_ow_bus, &iter) != ESP_OK) {
        ESP_LOGE(TAG_SENSOR, "1-Wire iter create failed — skipping DS18B20");
        return;
    }

    /* Discover every probe first, keeping handle + ROM together. */
    struct { ds18b20_device_handle_t h; uint64_t rom; } found[DS18B20_MAX];
    int found_count = 0;

    onewire_device_t device;
    esp_err_t err;
    do {
        err = onewire_device_iter_get_next(iter, &device);
        if (err == ESP_OK && found_count < DS18B20_MAX) {
            ds18b20_config_t ds_cfg = {};
            ds18b20_device_handle_t h;
            if (ds18b20_new_device(&device, &ds_cfg, &h) == ESP_OK) {
                found[found_count].h   = h;
                found[found_count].rom = device.address;
                ESP_LOGI(TAG_SENSOR, "DS18B20 discovered ROM: %016llX", device.address);
                found_count++;
            }
        }
    } while (err != ESP_ERR_NOT_FOUND);
    onewire_del_device_iter(iter);

    bool used[DS18B20_MAX] = { false };

    /* Pass 1: assign pinned roles by matching ROM. */
    for (int r = 0; r < DS18B20_MAX; r++) {
        if (role_rom[r] == 0) continue;
        for (int i = 0; i < found_count; i++) {
            if (!used[i] && found[i].rom == role_rom[r]) {
                s_ds18b20[r] = found[i].h;
                s_ds18b20_present[r] = true;
                used[i] = true;
                break;
            }
        }
        if (!s_ds18b20_present[r]) {
            ESP_LOGW(TAG_SENSOR, "%s ROM %016llX not on bus",
                     r == DS18B20_ROLE_WATER ? "water" : "external", role_rom[r]);
        }
    }

    /* Pass 2: fallback for any unpinned/missing role — assign remaining probe
     * by discovery order, but WARN loudly so a swap can't hide. */
    for (int r = 0; r < DS18B20_MAX; r++) {
        if (s_ds18b20_present[r]) continue;
        for (int i = 0; i < found_count; i++) {
            if (!used[i]) {
                s_ds18b20[r] = found[i].h;
                s_ds18b20_present[r] = true;
                used[i] = true;
                ESP_LOGW(TAG_SENSOR,
                    "%s role UNPINNED — using ROM %016llX by order; "
                    "set CONFIG_DS18B20_%s_ROM to pin it",
                    r == DS18B20_ROLE_WATER ? "water" : "external", found[i].rom,
                    r == DS18B20_ROLE_WATER ? "WATER" : "EXTERNAL");
                break;
            }
        }
    }

    ESP_LOGI(TAG_SENSOR, "DS18B20 roles: water=%s external=%s",
             s_ds18b20_present[DS18B20_ROLE_WATER]    ? "ready" : "MISSING",
             s_ds18b20_present[DS18B20_ROLE_EXTERNAL] ? "ready" : "MISSING");
}

void ds18b20_read_sensor(int role, sensor_reading_t *result)
{
    result->valid = false;
    strlcpy(result->status, "not_detected", sizeof(result->status));

    if (role < 0 || role >= DS18B20_MAX || !s_ds18b20_present[role]) return;

    ds18b20_trigger_temperature_conversion(s_ds18b20[role]);
    vTaskDelay(pdMS_TO_TICKS(800));  /* DS18B20 max conversion time at 12-bit */

    float temp;
    if (ds18b20_get_temperature(s_ds18b20[role], &temp) != ESP_OK) {
        strlcpy(result->status, "read_error", sizeof(result->status));
        return;
    }

    /* Plausibility is the validator's job — emit the raw value. */
    result->value = temp;
    result->valid = true;
    strlcpy(result->status, "ok", sizeof(result->status));
}
