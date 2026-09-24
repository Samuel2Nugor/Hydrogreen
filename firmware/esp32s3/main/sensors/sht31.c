/*
 * MicroHydros firmware — SHT31 (I2C) temperature + humidity driver.
 */
#include "sht31.h"

#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/i2c_master.h"

#define SHT31_I2C_ADDR   0x44
#define SHT31_CMD_MEAS_H 0x2C   /* high repeatability, clock stretch */
#define SHT31_CMD_MEAS_L 0x06

#define I2C_SDA CONFIG_I2C_SDA_GPIO
#define I2C_SCL CONFIG_I2C_SCL_GPIO

static const char *TAG_SENSOR = "sensor";

static i2c_master_bus_handle_t s_i2c_bus = NULL;
static i2c_master_dev_handle_t s_sht31_dev = NULL;
static bool                    s_sht31_ok = false;

static uint8_t sht31_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

void sht31_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port      = I2C_NUM_0,
        .sda_io_num    = I2C_SDA,
        .scl_io_num    = I2C_SCL,
        .clk_source    = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    if (i2c_new_master_bus(&bus_cfg, &s_i2c_bus) != ESP_OK) {
        ESP_LOGE(TAG_SENSOR, "SHT31: I2C bus init failed");
        return;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = SHT31_I2C_ADDR,
        .scl_speed_hz    = 100000,
    };
    if (i2c_master_bus_add_device(s_i2c_bus, &dev_cfg, &s_sht31_dev) != ESP_OK) {
        ESP_LOGE(TAG_SENSOR, "SHT31: device add failed");
        return;
    }

    /* probe */
    uint8_t cmd[2] = {SHT31_CMD_MEAS_H, SHT31_CMD_MEAS_L};
    uint8_t buf[6];
    esp_err_t err = i2c_master_transmit(s_sht31_dev, cmd, 2, 100);
    if (err == ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(20));
        err = i2c_master_receive(s_sht31_dev, buf, 6, 100);
    }
    s_sht31_ok = (err == ESP_OK);
    ESP_LOGI(TAG_SENSOR, "SHT31 @0x44: %s", s_sht31_ok ? "detected" : "NOT detected");
}

void sht31_read(sensor_reading_t *temp, sensor_reading_t *humidity)
{
    temp->valid = false;  humidity->valid = false;
    strlcpy(temp->status,     "not_detected", sizeof(temp->status));
    strlcpy(humidity->status, "not_detected", sizeof(humidity->status));

    if (!s_sht31_ok) return;

    uint8_t cmd[2] = {SHT31_CMD_MEAS_H, SHT31_CMD_MEAS_L};
    uint8_t buf[6];

    if (i2c_master_transmit(s_sht31_dev, cmd, 2, 100) != ESP_OK) goto read_error;
    vTaskDelay(pdMS_TO_TICKS(20));
    if (i2c_master_receive(s_sht31_dev, buf, 6, 100) != ESP_OK)  goto read_error;
    if (sht31_crc8(&buf[0], 2) != buf[2] ||
        sht31_crc8(&buf[3], 2) != buf[5])                        goto read_error;

    uint16_t raw_t = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t raw_h = ((uint16_t)buf[3] << 8) | buf[4];

    /* Transaction + CRC ok → both channels "ok". Plausibility is the
     * validator's job. Folding it in here would let an implausible temperature
     * reject a valid humidity reading (they share one sensor_status field). */
    temp->value     = -45.0f + 175.0f * raw_t / 65535.0f;
    humidity->value = 100.0f * raw_h / 65535.0f;
    temp->valid = true;  humidity->valid = true;
    strlcpy(temp->status,     "ok", sizeof(temp->status));
    strlcpy(humidity->status, "ok", sizeof(humidity->status));
    return;

read_error:
    strlcpy(temp->status,     "read_error", sizeof(temp->status));
    strlcpy(humidity->status, "read_error", sizeof(humidity->status));
}
