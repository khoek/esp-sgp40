#include <device/sensirion.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <math.h>

#include "device/sgp40.h"

static const char* TAG = "sgp40";

struct sgp40_cmd_exec {
    sensirion_cmd_def_t def;
};

struct sgp40_cmd_read {
    sensirion_cmd_def_t def;
};

struct sgp40_cmd_readwrite {
    sensirion_cmd_def_t def;
};

const sgp40_cmd_readwrite_t SGP40_CMD_MEASURE_RAW = {
    .def = {.code = 0x260F, .delay_ms = 30}};
const sgp40_cmd_read_t SGP40_CMD_MEASURE_TEST = {
    .def = {.code = 0x280E, .delay_ms = 250}};
const sgp40_cmd_exec_t SGP40_CMD_HEATER_OFF = {
    .def = {.code = 0x3615, .delay_ms = 1}};

// Undocumented command! (See SDK implementation.) Returns 3 16-bit words.
// Format: Big endian representation of 48-bit number.
const sgp40_cmd_read_t SGP40_CMD_GET_SERIAL_ID = {
    .def = {.code = 0x3682, .delay_ms = 1}};
// Undocumented command! (See SDK implementation.) Returns 1 16-bit words.
// Format: Unknown.
const sgp40_cmd_read_t SGP40_CMD_GET_FEATURESET = {
    .def = {.code = 0x202F, .delay_ms = 1}};

esp_err_t sgp40_init(i2c_port_t port, uint8_t addr, sgp40_handle_t* out_dev) {
    esp_err_t ret;

    sgp40_handle_t dev;
    sensirion_init(port, addr, &dev);

    // As per spec start up time is 0.6ms.
    vTaskDelay(1 + (1 / portTICK_PERIOD_MS));

    uint16_t serial[3];
    ret = sgp40_cmd_read(dev, &SGP40_CMD_GET_SERIAL_ID, serial, 3);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG,
                 "I2C read failed (0x%X), are I2C pin numbers/address correct?",
                 ret);
        goto sgp40_init_fail;
    }

    ESP_LOGD(TAG, "serial={0x%04X,0x%04X,0x%04X}", serial[0], serial[1],
             serial[2]);

    uint16_t featureset;
    ESP_ERROR_CHECK(
        sgp40_cmd_read(dev, &SGP40_CMD_GET_FEATURESET, &featureset, 1));
    ESP_LOGD(TAG, "featureset=0x%04X", featureset);

    *out_dev = dev;
    return ESP_OK;

sgp40_init_fail:
    sgp40_destroy(dev);
    return ret;
}

void sgp40_destroy(sgp40_handle_t dev) { sensirion_destroy(dev); }

esp_err_t sgp40_cmd_exec(sgp40_handle_t dev, const sgp40_cmd_exec_t* cmd) {
    return sensirion_cmd_perform(dev, &cmd->def, NULL, 0, NULL, 0);
}

esp_err_t sgp40_cmd_read(sgp40_handle_t dev, const sgp40_cmd_read_t* cmd,
                         uint16_t* in_data, size_t in_count) {
    return sensirion_cmd_perform(dev, &cmd->def, NULL, 0, in_data, in_count);
}

esp_err_t sgp40_cmd_readwrite(sgp40_handle_t dev,
                              const sgp40_cmd_readwrite_t* cmd,
                              const uint16_t* out_data, size_t out_count,
                              uint16_t* in_data, size_t in_count) {
    return sensirion_cmd_perform(dev, &cmd->def, out_data, out_count, in_data,
                                 in_count);
}

void sgp40_encode_temp(double temp_c, uint16_t* raw_temp) {
    int32_t val = round((temp_c + 45.0) * 65535.0 / 175.0);
    assert(val > 0 && val <= UINT16_MAX);
    *raw_temp = (uint16_t) val;
}

void sgp40_encode_hum(double rel_humidity, uint16_t* raw_hum) {
    int32_t val = round(rel_humidity * 65535.0 / 100.0);
    assert(val > 0 && val <= UINT16_MAX);
    *raw_hum = (uint16_t) val;
}
