/*
 * MicroHydros firmware — ESP32-S3 DevKitC-1.1
 * Framework : ESP-IDF v5.x
 *
 * Sensors
 *   SHT31   — I2C  (GPIO8=SDA, GPIO9=SCL)  → internal temp + humidity
 *   DS18B20 — 1-Wire (GPIO4, 4.7kΩ pull-up) → water temp + external air temp
 *
 * Data flow
 *   Reads sensors every CONFIG_TELEMETRY_INTERVAL_SEC seconds.
 *   Publishes one raw telemetry JSON to MQTT per data contract v1.
 *   Topic: microhydros/v1/devices/{device_id}/telemetry/raw
 *
 * Credentials
 *   Set WiFi SSID/password and MQTT broker URL via: idf.py menuconfig
 *   → MicroHydros Configuration
 *
 * Layout: wifi.c (station + reconnect), mqtt.c (client, LWT, telemetry
 * payload), sensors/sht31.c and sensors/ds18b20_roles.c (drivers). This file
 * wires them together and runs the telemetry loop.
 */

#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "mqtt.h"
#include "sensor_types.h"
#include "sht31.h"
#include "ds18b20_roles.h"

static const char *TAG        = "microhydros";
static const char *TAG_SENSOR = "sensor";

#define TELEMETRY_INTERVAL_US (CONFIG_TELEMETRY_INTERVAL_SEC * 1000ULL * 1000ULL)

/* Plausibility ranges live in Node-RED (PLAUSIBLE_RANGES) — single source of
 * truth. Firmware reports only transaction status (ok / read_error /
 * not_detected); the validator rejects out-of-range values per field. */

/* ═══════════════════════════════════════════════════════════════════════════
 * Telemetry task — sensor work runs here, not in esp_timer callback,
 * to avoid blocking the timer task with DS18B20 conversion delays (~1.6 s).
 * ═════════════════════════════════════════════════════════════════════════ */

static TaskHandle_t s_telemetry_task = NULL;

static void telemetry_task(void *arg)
{
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        sensor_reading_t internal_temp, internal_hum, water_temp, external_temp;

        sht31_read(&internal_temp, &internal_hum);
        ds18b20_read_sensor(DS18B20_ROLE_WATER,    &water_temp);
        ds18b20_read_sensor(DS18B20_ROLE_EXTERNAL, &external_temp);

        ESP_LOGI(TAG_SENSOR, "internal_temp=%.2f(%s) internal_hum=%.1f(%s) water=%.2f(%s) external=%.2f(%s)",
                 internal_temp.value,  internal_temp.status,
                 internal_hum.value,   internal_hum.status,
                 water_temp.value,     water_temp.status,
                 external_temp.value,  external_temp.status);

        mqtt_publish_telemetry(&internal_temp, &internal_hum, &water_temp, &external_temp);
    }
}

static void telemetry_timer_cb(void *arg)
{
    if (s_telemetry_task) {
        xTaskNotifyGive(s_telemetry_task);
    }
}

void app_main(void)
{
    /* Generate boot_id from random 32-bit value */
    char boot_id[9];
    snprintf(boot_id, sizeof(boot_id), "%08"PRIx32, esp_random());
    ESP_LOGI(TAG, "MicroHydros boot_id=%s device_id=%s", boot_id, CONFIG_DEVICE_ID);

    /* NVS */
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_err);

    /* Sensors */
    sht31_init();
    ds18b20_init_sensors();

    /* Network */
    wifi_init();
    mqtt_init(boot_id);
    mqtt_wait_connected(60000);

    /* Sensor task — receives notify from timer, does the actual work */
    xTaskCreate(telemetry_task, "telemetry", 4096, NULL, 5, &s_telemetry_task);

    /* Periodic telemetry timer */
    const esp_timer_create_args_t timer_args = {
        .callback = telemetry_timer_cb,
        .name     = "telemetry",
    };
    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, TELEMETRY_INTERVAL_US));

    /* Trigger first reading immediately */
    telemetry_timer_cb(NULL);

    ESP_LOGI(TAG, "running — publishing every %d s", CONFIG_TELEMETRY_INTERVAL_SEC);
}
