#pragma once

#include <device/sensirion.h>
#include <driver/i2c.h>

typedef struct sgp40_cmd_exec sgp40_cmd_exec_t;
typedef struct sgp40_cmd_read sgp40_cmd_read_t;
typedef struct sgp40_cmd_readwrite sgp40_cmd_readwrite_t;

extern const sgp40_cmd_readwrite_t SGP40_CMD_MEASURE_RAW;
extern const sgp40_cmd_read_t SGP40_CMD_MEASURE_TEST;
extern const sgp40_cmd_exec_t SGP40_CMD_HEATER_OFF;

// Undocumented command! (See SDK implementation.) Returns 3 16-bit words.
// Format: Big endian representation of 48-bit number.
extern const sgp40_cmd_read_t SGP40_CMD_GET_SERIAL_ID;
// Undocumented command! (See SDK implementation.) Returns 1 16-bit word.
// Format: Unknown.
extern const sgp40_cmd_read_t SGP40_CMD_GET_FEATURESET;

#define SGP40_MEASURE_TEST_PASS 0xD400
#define SGP40_MEASURE_TEST_FAIL 0x4B00

#define SGP40_COMPENSATION_DEFAULT_RAW_HUMIDITY 0x8000
#define SGP40_COMPENSATION_DEFAULT_RAW_TEMP 0x6666

typedef sensirion_dev_handle_t sgp40_handle_t;

// Register the SGP40 on the given I2C bus.
__result_use_check esp_err_t sgp40_init(i2c_port_t port, uint8_t addr,
                                        sgp40_handle_t* out_dev);

// Release the given handle.
void sgp40_destroy(sgp40_handle_t dev);

// Perform a command over I2C. Use of these functions is thread-safe.
__result_use_check esp_err_t sgp40_cmd_exec(sgp40_handle_t dev,
                                            const sgp40_cmd_exec_t* cmd);
__result_use_check esp_err_t sgp40_cmd_read(sgp40_handle_t dev,
                                            const sgp40_cmd_read_t* cmd,
                                            uint16_t* in_data, size_t in_count);
__result_use_check esp_err_t
sgp40_cmd_readwrite(sgp40_handle_t dev, const sgp40_cmd_readwrite_t* cmd,
                    const uint16_t* out_data, size_t out_count,
                    uint16_t* in_data, size_t in_count);

// Encode a temperature value to be passed to the SGP40.
void sgp40_encode_temp(double temp_c, uint16_t* raw_temp);

// Encode a humidity value to be passed to the SGP40.
void sgp40_encode_hum(double rel_humidity, uint16_t* raw_hum);

// The current VOC algorithm library is commit
// 00768191892b2cc0d839ebf95998fc4a85b660c4 from
// https://github.com/Sensirion/embedded-sgp/.

typedef struct sgp40_voc_algorithm_data sgp40_voc_algorithm_data_t;
typedef sgp40_voc_algorithm_data_t* sgp40_voc_algorithm_ctx_t;

// Initialize a VOC algorithm context with default parameters.
void sgp40_voc_algorithm_ctx_init(sgp40_voc_algorithm_ctx_t* out_ctx);

// Free the given VOC algorithm context.
void sgp40_voc_algorithm_ctx_destroy(sgp40_voc_algorithm_ctx_t ctx);

// Update the VOC algorithm data with the given `sraw` value, returning the
// currently calculated `voc_index`. As per spec, should be called with a
// frequence of ~1Hz.
void sgp40_voc_algorithm_ctx_process(sgp40_voc_algorithm_ctx_t ctx,
                                     uint16_t sraw, int32_t* voc_index);
