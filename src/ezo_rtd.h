#ifndef EZO_RTD_H
#define EZO_RTD_H

#include "ezo_i2c.h"
#include "ezo_uart.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  EZO_RTD_SCALE_UNKNOWN = 0,
  EZO_RTD_SCALE_CELSIUS,
  EZO_RTD_SCALE_KELVIN,
  EZO_RTD_SCALE_FAHRENHEIT
} ezo_rtd_scale_t;

typedef struct {
  double temperature;
  ezo_rtd_scale_t scale;
} ezo_rtd_reading_t;

typedef struct {
  ezo_rtd_scale_t scale;
} ezo_rtd_scale_status_t;

typedef struct {
  uint8_t calibrated;
} ezo_rtd_calibration_status_t;

ezo_result_t ezo_rtd_parse_reading(const char *buffer,
                                   size_t buffer_len,
                                   ezo_rtd_scale_t scale,
                                   ezo_rtd_reading_t *reading_out);

ezo_result_t ezo_rtd_parse_scale(const char *buffer,
                                 size_t buffer_len,
                                 ezo_rtd_scale_status_t *status_out);

ezo_result_t ezo_rtd_parse_calibration_status(
    const char *buffer,
    size_t buffer_len,
    ezo_rtd_calibration_status_t *status_out);

ezo_result_t ezo_rtd_build_scale_command(char *buffer,
                                         size_t buffer_len,
                                         ezo_rtd_scale_t scale);

ezo_result_t ezo_rtd_build_calibration_command(char *buffer,
                                               size_t buffer_len,
                                               double reference_temperature,
                                               uint8_t decimals);

ezo_result_t ezo_rtd_send_read_i2c(ezo_i2c_device_t *device,
                                   ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_rtd_send_scale_query_i2c(ezo_i2c_device_t *device,
                                          ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_rtd_send_scale_set_i2c(ezo_i2c_device_t *device,
                                        ezo_rtd_scale_t scale,
                                        ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_rtd_send_calibration_query_i2c(ezo_i2c_device_t *device,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_rtd_read_response_i2c(ezo_i2c_device_t *device,
                                       ezo_rtd_scale_t scale,
                                       ezo_rtd_reading_t *reading_out);

ezo_result_t ezo_rtd_read_scale_i2c(ezo_i2c_device_t *device,
                                    ezo_rtd_scale_status_t *status_out);

ezo_result_t ezo_rtd_read_calibration_status_i2c(
    ezo_i2c_device_t *device,
    ezo_rtd_calibration_status_t *status_out);

ezo_result_t ezo_rtd_send_read_uart(ezo_uart_device_t *device,
                                    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_rtd_send_scale_query_uart(ezo_uart_device_t *device,
                                           ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_rtd_send_scale_set_uart(ezo_uart_device_t *device,
                                         ezo_rtd_scale_t scale,
                                         ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_rtd_send_calibration_query_uart(ezo_uart_device_t *device,
                                                 ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_rtd_read_response_uart(ezo_uart_device_t *device,
                                        ezo_rtd_scale_t scale,
                                        ezo_rtd_reading_t *reading_out);

ezo_result_t ezo_rtd_read_scale_uart(ezo_uart_device_t *device,
                                     ezo_rtd_scale_status_t *status_out);

ezo_result_t ezo_rtd_read_calibration_status_uart(
    ezo_uart_device_t *device,
    ezo_rtd_calibration_status_t *status_out);

#ifdef __cplusplus
}
#endif

#endif
