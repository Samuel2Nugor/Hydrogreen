/*
 * MicroHydros firmware — MQTT client, status LWT and telemetry payload.
 */
#include "mqtt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "mqtt_client.h"
#include "cJSON.h"

#define MQTT_BROKER_URL CONFIG_MQTT_BROKER_URL
#define DEVICE_ID       CONFIG_DEVICE_ID

static const char *TAG_MQTT = "mqtt";

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static bool                     s_mqtt_connected = false;
static char                     s_boot_id[9];      /* 8 hex chars + null */
static uint32_t                 s_sequence = 0;

static void build_status_payload(char *buf, size_t len, const char *status)
{
    snprintf(buf, len,
        "{\"schema_version\":1,\"device_id\":\"%s\","
        "\"boot_id\":\"%s\",\"status\":\"%s\"}",
        DEVICE_ID, s_boot_id, status);
}

static void mqtt_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    char status_topic[80];
    char status_payload[128];

    snprintf(status_topic, sizeof(status_topic),
             "microhydros/v1/devices/%s/status", DEVICE_ID);

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_MQTT, "connected to broker");
        s_mqtt_connected = true;
        build_status_payload(status_payload, sizeof(status_payload), "online");
        esp_mqtt_client_publish(s_mqtt_client, status_topic,
                                status_payload, 0, 1, 1);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG_MQTT, "disconnected");
        s_mqtt_connected = false;
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG_MQTT, "error");
        break;
    default:
        break;
    }
}

void mqtt_init(const char *boot_id)
{
    snprintf(s_boot_id, sizeof(s_boot_id), "%s", boot_id);

    char status_topic[80];
    char lwt_payload[128];

    snprintf(status_topic, sizeof(status_topic),
             "microhydros/v1/devices/%s/status", DEVICE_ID);
    build_status_payload(lwt_payload, sizeof(lwt_payload), "offline");

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri     = MQTT_BROKER_URL,
        .session.last_will = {
            .topic   = status_topic,
            .msg     = lwt_payload,
            .msg_len = strlen(lwt_payload),
            .qos     = 1,
            .retain  = 1,
        },
    };

    s_mqtt_client = esp_mqtt_client_init(&cfg);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(
        s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(s_mqtt_client));
}

bool mqtt_wait_connected(uint32_t timeout_ms)
{
    const uint32_t step_ms = 500;
    for (uint32_t waited = 0; waited < timeout_ms && !s_mqtt_connected; waited += step_ms) {
        vTaskDelay(pdMS_TO_TICKS(step_ms));
    }
    if (!s_mqtt_connected) {
        ESP_LOGW(TAG_MQTT, "not connected within %" PRIu32 "s — continuing, will retry in background",
                 timeout_ms / 1000);
    }
    return s_mqtt_connected;
}

void mqtt_publish_telemetry(const sensor_reading_t *internal_temp,
                            const sensor_reading_t *internal_hum,
                            const sensor_reading_t *water_temp,
                            const sensor_reading_t *external_temp)
{
    if (!s_mqtt_connected) {
        ESP_LOGW(TAG_MQTT, "not connected — skipping publish");
        return;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        ESP_LOGE(TAG_MQTT, "cJSON alloc failed — skipping publish");
        return;
    }
    cJSON_AddNumberToObject(root, "schema_version", 1);
    cJSON_AddStringToObject(root, "device_id",      DEVICE_ID);
    cJSON_AddStringToObject(root, "boot_id",        s_boot_id);
    cJSON_AddNumberToObject(root, "sequence",       s_sequence++);
    cJSON_AddNumberToObject(root, "uptime_ms",      (double)(esp_timer_get_time() / 1000));

    cJSON *measurements = cJSON_AddObjectToObject(root, "measurements");
    if (internal_temp->valid)
        cJSON_AddNumberToObject(measurements, "internal_temperature_c",  internal_temp->value);
    else
        cJSON_AddNullToObject(measurements,  "internal_temperature_c");

    if (internal_hum->valid)
        cJSON_AddNumberToObject(measurements, "internal_humidity_percent", internal_hum->value);
    else
        cJSON_AddNullToObject(measurements,   "internal_humidity_percent");

    if (external_temp->valid)
        cJSON_AddNumberToObject(measurements, "external_temperature_c",  external_temp->value);
    else
        cJSON_AddNullToObject(measurements,   "external_temperature_c");

    if (water_temp->valid)
        cJSON_AddNumberToObject(measurements, "water_temperature_c",     water_temp->value);
    else
        cJSON_AddNullToObject(measurements,   "water_temperature_c");

    cJSON *sensor_status = cJSON_AddObjectToObject(root, "sensor_status");
    cJSON_AddStringToObject(sensor_status, "internal_sht31",    internal_temp->status);
    cJSON_AddStringToObject(sensor_status, "water_ds18b20",     water_temp->status);
    cJSON_AddStringToObject(sensor_status, "external_ds18b20",  external_temp->status);

    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload) {
        ESP_LOGE(TAG_MQTT, "cJSON print failed — skipping publish");
        return;
    }

    char topic[80];
    snprintf(topic, sizeof(topic),
             "microhydros/v1/devices/%s/telemetry/raw", DEVICE_ID);

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG_MQTT, "published (msg_id=%d) seq=%"PRIu32, msg_id, s_sequence - 1);
    ESP_LOGI(TAG_MQTT, "%s", payload);

    free(payload);
}
