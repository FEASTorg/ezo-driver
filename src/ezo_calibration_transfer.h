#ifndef EZO_CALIBRATION_TRANSFER_H
#define EZO_CALIBRATION_TRANSFER_H

#include "ezo_i2c.h"
#include "ezo_product.h"
#include "ezo_uart.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t chunk_count;
  uint32_t byte_count;
} ezo_calibration_export_info_t;

typedef struct {
  ezo_device_status_t device_status;
  uint8_t pending_reboot;
} ezo_calibration_import_result_t;

ezo_result_t ezo_calibration_parse_export_info(
    const char *buffer,
    size_t buffer_len,
    ezo_calibration_export_info_t *info_out);

ezo_result_t ezo_calibration_build_import_command(char *buffer,
                                                  size_t buffer_len,
                                                  const char *payload);

ezo_result_t ezo_calibration_send_export_info_query_i2c(
    ezo_i2c_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_calibration_send_export_next_i2c(ezo_i2c_device_t *device,
                                                  ezo_product_id_t product_id,
                                                  ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_calibration_send_import_i2c(ezo_i2c_device_t *device,
                                             ezo_product_id_t product_id,
                                             const char *payload,
                                             ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_calibration_read_export_info_i2c(
    ezo_i2c_device_t *device,
    ezo_calibration_export_info_t *info_out);

ezo_result_t ezo_calibration_read_export_chunk_i2c(ezo_i2c_device_t *device,
                                                   char *buffer,
                                                   size_t buffer_len,
                                                   size_t *chunk_len_out);

ezo_result_t ezo_calibration_read_import_status_i2c(ezo_i2c_device_t *device,
                                                    ezo_device_status_t *status_out);

ezo_result_t ezo_calibration_read_import_result_i2c(
    ezo_i2c_device_t *device,
    ezo_calibration_import_result_t *result_out);

ezo_result_t ezo_calibration_send_export_info_query_uart(
    ezo_uart_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_calibration_send_export_next_uart(ezo_uart_device_t *device,
                                                   ezo_product_id_t product_id,
                                                   ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_calibration_send_import_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id,
                                              const char *payload,
                                              ezo_timing_hint_t *timing_hint);

ezo_result_t ezo_calibration_read_export_info_uart(
    ezo_uart_device_t *device,
    ezo_calibration_export_info_t *info_out);

ezo_result_t ezo_calibration_read_export_chunk_uart(
    ezo_uart_device_t *device,
    char *buffer,
    size_t buffer_len,
    size_t *chunk_len_out,
    ezo_uart_response_kind_t *response_kind_out);

ezo_result_t ezo_calibration_read_import_result_uart(
    ezo_uart_device_t *device,
    ezo_uart_response_kind_t *response_kind_out);

#ifdef __cplusplus
}
#endif

#endif
