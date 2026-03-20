#ifndef EZO_ORP_H
#define EZO_ORP_H

#include "ezo_i2c.h"
#include "ezo_uart.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  EZO_ORP_EXTENDED_SCALE_DISABLED = 0,
  EZO_ORP_EXTENDED_SCALE_ENABLED = 1
} ezo_orp_extended_scale_t;

typedef struct {
  double millivolts;
} ezo_orp_reading_t;

typedef struct {
  uint8_t calibrated;
} ezo_orp_calibration_status_t;

typedef struct {
  ezo_orp_extended_scale_t enabled;
} ezo_orp_extended_scale_status_t;

ezo_result_t ezo_orp_parse_reading(const char *buffer,
                                   size_t buffer_len,
                                   ezo_orp_reading_t *reading_out);

ezo_result_t ezo_orp_parse_calibration_status(
    const char *buffer,
    size_t buffer_len,
    ezo_orp_calibration_status_t *status_out);

ezo_result_t ezo_orp_parse_extended_scale(
    const char *buffer,
    size_t buffer_len,
    ezo_orp_extended_scale_status_t *status_out);

ezo_result_t ezo_orp_build_calibration_command(char *buffer,
                                               size_t buffer_len,
                                               double reference_mv,
                                               uint8_t decimals);

ezo_result_t ezo_orp_build_extended_scale_command(
    char *buffer,
    size_t buffer_len,
    ezo_orp_extended_scale_t extended_scale);

ezo_result_t ezo_orp_send_read_i2c(ezo_i2c_device_t *device,
                                   ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_calibration_query_i2c(ezo_i2c_device_t *device,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_calibration_i2c(ezo_i2c_device_t *device,
                                          double reference_mv,
                                          uint8_t decimals,
                                          ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_clear_calibration_i2c(ezo_i2c_device_t *device,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_extended_scale_query_i2c(ezo_i2c_device_t *device,
                                                   ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_extended_scale_set_i2c(
    ezo_i2c_device_t *device,
    ezo_orp_extended_scale_t extended_scale,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_read_response_i2c(ezo_i2c_device_t *device,
                                       ezo_orp_reading_t *reading_out);

ezo_result_t ezo_orp_read_calibration_status_i2c(
    ezo_i2c_device_t *device,
    ezo_orp_calibration_status_t *status_out);

ezo_result_t ezo_orp_read_extended_scale_i2c(
    ezo_i2c_device_t *device,
    ezo_orp_extended_scale_status_t *status_out);

ezo_result_t ezo_orp_send_read_uart(ezo_uart_device_t *device,
                                    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_calibration_query_uart(ezo_uart_device_t *device,
                                                 ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_calibration_uart(ezo_uart_device_t *device,
                                           double reference_mv,
                                           uint8_t decimals,
                                           ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_clear_calibration_uart(ezo_uart_device_t *device,
                                                 ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_extended_scale_query_uart(
    ezo_uart_device_t *device,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_send_extended_scale_set_uart(
    ezo_uart_device_t *device,
    ezo_orp_extended_scale_t extended_scale,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_orp_read_response_uart(ezo_uart_device_t *device,
                                        ezo_orp_reading_t *reading_out);

ezo_result_t ezo_orp_read_calibration_status_uart(
    ezo_uart_device_t *device,
    ezo_orp_calibration_status_t *status_out);

ezo_result_t ezo_orp_read_extended_scale_uart(
    ezo_uart_device_t *device,
    ezo_orp_extended_scale_status_t *status_out);

#ifdef __cplusplus
}
#endif

#endif
