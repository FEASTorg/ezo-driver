#ifndef EZO_CONTROL_H
#define EZO_CONTROL_H

#include "ezo_i2c.h"
#include "ezo_product.h"
#include "ezo_uart.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EZO_CONTROL_NAME_MAX_LEN 32

typedef struct {
  char name[EZO_CONTROL_NAME_MAX_LEN];
} ezo_control_name_t;

typedef struct {
  char restart_code;
  double supply_voltage;
} ezo_control_status_t;

typedef struct {
  uint8_t enabled;
} ezo_control_led_status_t;

typedef struct {
  uint8_t enabled;
} ezo_control_protocol_lock_status_t;

typedef struct {
  uint32_t baud_rate;
} ezo_control_baud_status_t;

typedef struct {
  uint8_t enabled;
} ezo_control_response_code_status_t;

ezo_result_t ezo_control_parse_name(const char *buffer,
                                    size_t buffer_len,
                                    ezo_control_name_t *name_out);

ezo_result_t ezo_control_parse_status(const char *buffer,
                                      size_t buffer_len,
                                      ezo_control_status_t *status_out);

ezo_result_t ezo_control_parse_led(const char *buffer,
                                   size_t buffer_len,
                                   ezo_control_led_status_t *status_out);

ezo_result_t ezo_control_parse_protocol_lock(
    const char *buffer,
    size_t buffer_len,
    ezo_control_protocol_lock_status_t *status_out);

ezo_result_t ezo_control_parse_baud(const char *buffer,
                                    size_t buffer_len,
                                    ezo_control_baud_status_t *status_out);

ezo_result_t ezo_control_parse_response_code(
    const char *buffer,
    size_t buffer_len,
    ezo_control_response_code_status_t *status_out);

ezo_result_t ezo_control_build_name_command(char *buffer,
                                            size_t buffer_len,
                                            const char *name);

ezo_result_t ezo_control_build_led_command(char *buffer,
                                           size_t buffer_len,
                                           uint8_t enabled);

ezo_result_t ezo_control_build_protocol_lock_command(char *buffer,
                                                     size_t buffer_len,
                                                     uint8_t enabled);

ezo_result_t ezo_control_build_switch_to_i2c_command(char *buffer,
                                                     size_t buffer_len,
                                                     uint8_t i2c_address);

ezo_result_t ezo_control_build_switch_to_uart_command(char *buffer,
                                                      size_t buffer_len,
                                                      uint32_t baud_rate);

ezo_result_t ezo_control_build_response_code_command(char *buffer,
                                                     size_t buffer_len,
                                                     uint8_t enabled);

ezo_result_t ezo_control_send_info_query_i2c(ezo_i2c_device_t *device,
                                             ezo_product_id_t product_id,
                                             ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_name_query_i2c(ezo_i2c_device_t *device,
                                             ezo_product_id_t product_id,
                                             ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_name_set_i2c(ezo_i2c_device_t *device,
                                           ezo_product_id_t product_id,
                                           const char *name,
                                           ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_status_query_i2c(ezo_i2c_device_t *device,
                                               ezo_product_id_t product_id,
                                               ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_led_query_i2c(ezo_i2c_device_t *device,
                                            ezo_product_id_t product_id,
                                            ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_led_set_i2c(ezo_i2c_device_t *device,
                                          ezo_product_id_t product_id,
                                          uint8_t enabled,
                                          ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_find_i2c(ezo_i2c_device_t *device,
                                       ezo_product_id_t product_id,
                                       ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_sleep_i2c(ezo_i2c_device_t *device,
                                        ezo_product_id_t product_id,
                                        ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_factory_reset_i2c(ezo_i2c_device_t *device,
                                                ezo_product_id_t product_id,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_protocol_lock_query_i2c(
    ezo_i2c_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_protocol_lock_set_i2c(
    ezo_i2c_device_t *device,
    ezo_product_id_t product_id,
    uint8_t enabled,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_switch_to_uart_i2c(ezo_i2c_device_t *device,
                                                 ezo_product_id_t product_id,
                                                 uint32_t baud_rate,
                                                 ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_read_info_i2c(ezo_i2c_device_t *device,
                                       ezo_device_info_t *info_out);

ezo_result_t ezo_control_read_name_i2c(ezo_i2c_device_t *device,
                                       ezo_control_name_t *name_out);

ezo_result_t ezo_control_read_status_i2c(ezo_i2c_device_t *device,
                                         ezo_control_status_t *status_out);

ezo_result_t ezo_control_read_led_i2c(ezo_i2c_device_t *device,
                                      ezo_control_led_status_t *status_out);

ezo_result_t ezo_control_read_protocol_lock_i2c(
    ezo_i2c_device_t *device,
    ezo_control_protocol_lock_status_t *status_out);

ezo_result_t ezo_control_send_info_query_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id,
                                              ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_name_query_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id,
                                              ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_name_set_uart(ezo_uart_device_t *device,
                                            ezo_product_id_t product_id,
                                            const char *name,
                                            ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_status_query_uart(ezo_uart_device_t *device,
                                                ezo_product_id_t product_id,
                                                ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_led_query_uart(ezo_uart_device_t *device,
                                             ezo_product_id_t product_id,
                                             ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_led_set_uart(ezo_uart_device_t *device,
                                           ezo_product_id_t product_id,
                                           uint8_t enabled,
                                           ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_find_uart(ezo_uart_device_t *device,
                                        ezo_product_id_t product_id,
                                        ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_sleep_uart(ezo_uart_device_t *device,
                                         ezo_product_id_t product_id,
                                         ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_factory_reset_uart(ezo_uart_device_t *device,
                                                 ezo_product_id_t product_id,
                                                 ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_protocol_lock_query_uart(
    ezo_uart_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_protocol_lock_set_uart(
    ezo_uart_device_t *device,
    ezo_product_id_t product_id,
    uint8_t enabled,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_baud_query_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id,
                                              ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_response_code_query_uart(
    ezo_uart_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_response_code_set_uart(ezo_uart_device_t *device,
                                                     ezo_product_id_t product_id,
                                                     uint8_t enabled,
                                                     ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_switch_to_i2c_uart(ezo_uart_device_t *device,
                                                 ezo_product_id_t product_id,
                                                 uint8_t i2c_address,
                                                 ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_send_switch_to_uart_uart(ezo_uart_device_t *device,
                                                  ezo_product_id_t product_id,
                                                  uint32_t baud_rate,
                                                  ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_control_read_info_uart(ezo_uart_device_t *device,
                                        ezo_device_info_t *info_out);

ezo_result_t ezo_control_read_name_uart(ezo_uart_device_t *device,
                                        ezo_control_name_t *name_out);

ezo_result_t ezo_control_read_status_uart(ezo_uart_device_t *device,
                                          ezo_control_status_t *status_out);

ezo_result_t ezo_control_read_led_uart(ezo_uart_device_t *device,
                                       ezo_control_led_status_t *status_out);

ezo_result_t ezo_control_read_protocol_lock_uart(
    ezo_uart_device_t *device,
    ezo_control_protocol_lock_status_t *status_out);

ezo_result_t ezo_control_read_baud_uart(ezo_uart_device_t *device,
                                        ezo_control_baud_status_t *status_out);

ezo_result_t ezo_control_read_response_code_uart(
    ezo_uart_device_t *device,
    ezo_control_response_code_status_t *status_out);

#ifdef __cplusplus
}
#endif

#endif
