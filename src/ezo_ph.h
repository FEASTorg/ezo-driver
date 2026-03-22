#ifndef EZO_PH_H
#define EZO_PH_H

#include "ezo_i2c.h"
#include "ezo_uart.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  EZO_PH_CALIBRATION_NONE = 0,
  EZO_PH_CALIBRATION_ONE_POINT = 1,
  EZO_PH_CALIBRATION_TWO_POINT = 2,
  EZO_PH_CALIBRATION_THREE_POINT = 3
} ezo_ph_calibration_level_t;

typedef enum {
  EZO_PH_CALIBRATION_POINT_MID = 0,
  EZO_PH_CALIBRATION_POINT_LOW,
  EZO_PH_CALIBRATION_POINT_HIGH
} ezo_ph_calibration_point_t;

typedef enum {
  EZO_PH_EXTENDED_RANGE_DISABLED = 0,
  EZO_PH_EXTENDED_RANGE_ENABLED = 1
} ezo_ph_extended_range_t;

typedef struct {
  double ph;
} ezo_ph_reading_t;

typedef struct {
  double temperature_c;
} ezo_ph_temperature_compensation_t;

typedef struct {
  ezo_ph_calibration_level_t level;
} ezo_ph_calibration_status_t;

typedef struct {
  double acid_percent;
  double base_percent;
  double neutral_mv;
} ezo_ph_slope_t;

typedef struct {
  ezo_ph_extended_range_t enabled;
} ezo_ph_extended_range_status_t;

ezo_result_t ezo_ph_parse_reading(const char *buffer,
                                  size_t buffer_len,
                                  ezo_ph_reading_t *reading_out);

ezo_result_t ezo_ph_parse_temperature(const char *buffer,
                                      size_t buffer_len,
                                      ezo_ph_temperature_compensation_t *temperature_out);

ezo_result_t ezo_ph_parse_calibration_status(const char *buffer,
                                             size_t buffer_len,
                                             ezo_ph_calibration_status_t *status_out);

ezo_result_t ezo_ph_parse_slope(const char *buffer,
                                size_t buffer_len,
                                ezo_ph_slope_t *slope_out);

ezo_result_t ezo_ph_parse_extended_range(const char *buffer,
                                         size_t buffer_len,
                                         ezo_ph_extended_range_status_t *status_out);

ezo_result_t ezo_ph_build_temperature_command(char *buffer,
                                              size_t buffer_len,
                                              double temperature_c,
                                              uint8_t decimals);

ezo_result_t ezo_ph_build_calibration_command(char *buffer,
                                              size_t buffer_len,
                                              ezo_ph_calibration_point_t point,
                                              double reference_ph,
                                              uint8_t decimals);

ezo_result_t ezo_ph_build_extended_range_command(char *buffer,
                                                 size_t buffer_len,
                                                 ezo_ph_extended_range_t enabled);

ezo_result_t ezo_ph_send_read_i2c(ezo_i2c_device_t *device,
                                  ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_read_with_temp_comp_i2c(ezo_i2c_device_t *device,
                                                 double temperature_c,
                                                 uint8_t decimals,
                                                 ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_temperature_query_i2c(ezo_i2c_device_t *device,
                                               ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_temperature_set_i2c(ezo_i2c_device_t *device,
                                             double temperature_c,
                                             uint8_t decimals,
                                             ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_calibration_query_i2c(ezo_i2c_device_t *device,
                                               ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_calibration_i2c(ezo_i2c_device_t *device,
                                         ezo_ph_calibration_point_t point,
                                         double reference_ph,
                                         uint8_t decimals,
                                         ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_clear_calibration_i2c(ezo_i2c_device_t *device,
                                               ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_slope_query_i2c(ezo_i2c_device_t *device,
                                         ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_extended_range_query_i2c(ezo_i2c_device_t *device,
                                                  ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_extended_range_set_i2c(ezo_i2c_device_t *device,
                                                ezo_ph_extended_range_t enabled,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_read_response_i2c(ezo_i2c_device_t *device,
                                      ezo_ph_reading_t *reading_out);

ezo_result_t ezo_ph_read_temperature_i2c(ezo_i2c_device_t *device,
                                         ezo_ph_temperature_compensation_t *temperature_out);

ezo_result_t ezo_ph_read_calibration_status_i2c(ezo_i2c_device_t *device,
                                                ezo_ph_calibration_status_t *status_out);

ezo_result_t ezo_ph_read_slope_i2c(ezo_i2c_device_t *device,
                                   ezo_ph_slope_t *slope_out);

ezo_result_t ezo_ph_read_extended_range_i2c(ezo_i2c_device_t *device,
                                            ezo_ph_extended_range_status_t *status_out);

ezo_result_t ezo_ph_send_read_uart(ezo_uart_device_t *device,
                                   ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_read_with_temp_comp_uart(ezo_uart_device_t *device,
                                                  double temperature_c,
                                                  uint8_t decimals,
                                                  ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_temperature_query_uart(ezo_uart_device_t *device,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_temperature_set_uart(ezo_uart_device_t *device,
                                              double temperature_c,
                                              uint8_t decimals,
                                              ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_calibration_query_uart(ezo_uart_device_t *device,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_calibration_uart(ezo_uart_device_t *device,
                                          ezo_ph_calibration_point_t point,
                                          double reference_ph,
                                          uint8_t decimals,
                                          ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_clear_calibration_uart(ezo_uart_device_t *device,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_slope_query_uart(ezo_uart_device_t *device,
                                          ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_extended_range_query_uart(ezo_uart_device_t *device,
                                                   ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_send_extended_range_set_uart(ezo_uart_device_t *device,
                                                 ezo_ph_extended_range_t enabled,
                                                 ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_ph_read_response_uart(ezo_uart_device_t *device,
                                       ezo_ph_reading_t *reading_out);

ezo_result_t ezo_ph_read_response_with_temp_comp_uart(
    ezo_uart_device_t *device,
    ezo_ph_reading_t *reading_out);

ezo_result_t ezo_ph_read_temperature_uart(
    ezo_uart_device_t *device,
    ezo_ph_temperature_compensation_t *temperature_out);

ezo_result_t ezo_ph_read_calibration_status_uart(
    ezo_uart_device_t *device,
    ezo_ph_calibration_status_t *status_out);

ezo_result_t ezo_ph_read_slope_uart(ezo_uart_device_t *device,
                                    ezo_ph_slope_t *slope_out);

ezo_result_t ezo_ph_read_extended_range_uart(ezo_uart_device_t *device,
                                             ezo_ph_extended_range_status_t *status_out);

#ifdef __cplusplus
}
#endif

#endif
