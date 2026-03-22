#include "ezo_calibration_transfer.h"

#include "ezo_parse.h"

#include <string.h>

enum {
  EZO_CALIBRATION_RESPONSE_BUFFER_LEN = 96
};

static ezo_result_t ezo_calibration_send_i2c_command(ezo_i2c_device_t *device,
                                                     ezo_product_id_t product_id,
                                                     const char *command,
                                                     ezo_timing_hint_t *timing_hint) {
  ezo_timing_hint_t local_hint;
  ezo_result_t result =
      ezo_product_resolve_timing_hint(product_id,
                                      EZO_PRODUCT_TRANSPORT_I2C,
                                      EZO_COMMAND_GENERIC,
                                      timing_hint != NULL ? timing_hint : &local_hint);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_send_command(device, command, EZO_COMMAND_GENERIC, NULL);
}

static ezo_result_t ezo_calibration_send_uart_command(ezo_uart_device_t *device,
                                                      ezo_product_id_t product_id,
                                                      const char *command,
                                                      ezo_timing_hint_t *timing_hint) {
  ezo_timing_hint_t local_hint;
  ezo_result_t result =
      ezo_product_resolve_timing_hint(product_id,
                                      EZO_PRODUCT_TRANSPORT_UART,
                                      EZO_COMMAND_GENERIC,
                                      timing_hint != NULL ? timing_hint : &local_hint);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_uart_send_command(device, command, EZO_COMMAND_GENERIC, NULL);
}

static ezo_result_t ezo_calibration_read_i2c_text(ezo_i2c_device_t *device,
                                                  char *buffer,
                                                  size_t buffer_len,
                                                  size_t *response_len) {
  ezo_device_status_t status = EZO_STATUS_UNKNOWN;
  ezo_result_t result = ezo_read_response(device, buffer, buffer_len, response_len, &status);
  if (result != EZO_OK) {
    return result;
  }

  if (status != EZO_STATUS_SUCCESS) {
    return EZO_ERR_PROTOCOL;
  }

  return EZO_OK;
}

static ezo_result_t ezo_calibration_read_uart_data_then_ok(
    ezo_uart_device_t *device,
    char *buffer,
    size_t buffer_len,
    size_t *response_len) {
  ezo_uart_response_kind_t response_kind = EZO_UART_RESPONSE_UNKNOWN;
  char status_buffer[8];
  size_t status_len = 0;
  ezo_result_t result =
      ezo_uart_read_line(device, buffer, buffer_len, response_len, &response_kind);
  if (result != EZO_OK) {
    return result;
  }

  if (response_kind != EZO_UART_RESPONSE_DATA) {
    return EZO_ERR_PROTOCOL;
  }

  result = ezo_uart_read_line(device,
                              status_buffer,
                              sizeof(status_buffer),
                              &status_len,
                              &response_kind);
  if (result != EZO_OK) {
    return result;
  }

  return response_kind == EZO_UART_RESPONSE_OK ? EZO_OK : EZO_ERR_PROTOCOL;
}

ezo_result_t ezo_calibration_parse_export_info(
    const char *buffer,
    size_t buffer_len,
    ezo_calibration_export_info_t *info_out) {
  ezo_text_span_t fields[2];
  size_t field_count = 0;
  ezo_result_t result = EZO_OK;

  if (info_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  result = ezo_parse_csv_fields(buffer, buffer_len, fields, 2, &field_count);
  if (result != EZO_OK) {
    return result;
  }

  if (field_count != 2) {
    return EZO_ERR_PARSE;
  }

  result = ezo_parse_text_span_uint32(fields[0], &info_out->chunk_count);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_parse_text_span_uint32(fields[1], &info_out->byte_count);
}

ezo_result_t ezo_calibration_build_import_command(char *buffer,
                                                  size_t buffer_len,
                                                  const char *payload) {
  static const char prefix[] = "Import,";
  size_t prefix_len = sizeof(prefix) - 1U;
  size_t payload_len = 0;

  if (buffer == NULL || buffer_len == 0 || payload == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  payload_len = strlen(payload);
  if (strchr(payload, '\r') != NULL || strchr(payload, '\n') != NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  if (prefix_len + payload_len + 1U > buffer_len) {
    return EZO_ERR_BUFFER_TOO_SMALL;
  }

  memcpy(buffer, prefix, prefix_len);
  if (payload_len > 0) {
    memcpy(buffer + prefix_len, payload, payload_len);
  }
  buffer[prefix_len + payload_len] = '\0';
  return EZO_OK;
}

ezo_result_t ezo_calibration_send_export_info_query_i2c(
    ezo_i2c_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint) {
  return ezo_calibration_send_i2c_command(device, product_id, "Export,?", timing_hint);
}

ezo_result_t ezo_calibration_send_export_next_i2c(ezo_i2c_device_t *device,
                                                  ezo_product_id_t product_id,
                                                  ezo_timing_hint_t *timing_hint) {
  return ezo_calibration_send_i2c_command(device, product_id, "Export", timing_hint);
}

ezo_result_t ezo_calibration_send_import_i2c(ezo_i2c_device_t *device,
                                             ezo_product_id_t product_id,
                                             const char *payload,
                                             ezo_timing_hint_t *timing_hint) {
  char command[EZO_CALIBRATION_RESPONSE_BUFFER_LEN];
  ezo_result_t result =
      ezo_calibration_build_import_command(command, sizeof(command), payload);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_calibration_send_i2c_command(device, product_id, command, timing_hint);
}

ezo_result_t ezo_calibration_read_export_info_i2c(
    ezo_i2c_device_t *device,
    ezo_calibration_export_info_t *info_out) {
  char buffer[EZO_CALIBRATION_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result =
      ezo_calibration_read_i2c_text(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_calibration_parse_export_info(buffer, response_len, info_out);
}

ezo_result_t ezo_calibration_read_export_chunk_i2c(ezo_i2c_device_t *device,
                                                   char *buffer,
                                                   size_t buffer_len,
                                                   size_t *chunk_len_out) {
  if (buffer == NULL || buffer_len == 0 || chunk_len_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  return ezo_calibration_read_i2c_text(device, buffer, buffer_len, chunk_len_out);
}

ezo_result_t ezo_calibration_read_import_status_i2c(ezo_i2c_device_t *device,
                                                    ezo_device_status_t *status_out) {
  ezo_calibration_import_result_t result;
  ezo_result_t read_result = EZO_OK;

  if (status_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  read_result = ezo_calibration_read_import_result_i2c(device, &result);
  if (read_result != EZO_OK) {
    return read_result;
  }

  *status_out = result.device_status;
  return EZO_OK;
}

ezo_result_t ezo_calibration_read_import_result_i2c(
    ezo_i2c_device_t *device,
    ezo_calibration_import_result_t *result_out) {
  char buffer[EZO_CALIBRATION_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result = EZO_OK;

  if (result_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  result_out->device_status = EZO_STATUS_UNKNOWN;
  result_out->pending_reboot = 0;

  result = ezo_read_response(device,
                             buffer,
                             sizeof(buffer),
                             &response_len,
                             &result_out->device_status);
  if (result != EZO_OK) {
    return result;
  }

  if (response_len == 0) {
    return EZO_OK;
  }

  if (response_len == strlen("*Pending") &&
      memcmp(buffer, "*Pending", strlen("*Pending")) == 0) {
    result_out->pending_reboot = 1;
    return EZO_OK;
  }

  return EZO_ERR_PROTOCOL;
}

ezo_result_t ezo_calibration_send_export_info_query_uart(
    ezo_uart_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint) {
  return ezo_calibration_send_uart_command(device, product_id, "Export,?", timing_hint);
}

ezo_result_t ezo_calibration_send_export_next_uart(ezo_uart_device_t *device,
                                                   ezo_product_id_t product_id,
                                                   ezo_timing_hint_t *timing_hint) {
  return ezo_calibration_send_uart_command(device, product_id, "Export", timing_hint);
}

ezo_result_t ezo_calibration_send_import_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id,
                                              const char *payload,
                                              ezo_timing_hint_t *timing_hint) {
  char command[EZO_CALIBRATION_RESPONSE_BUFFER_LEN];
  ezo_result_t result =
      ezo_calibration_build_import_command(command, sizeof(command), payload);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_calibration_send_uart_command(device, product_id, command, timing_hint);
}

ezo_result_t ezo_calibration_read_export_info_uart(
    ezo_uart_device_t *device,
    ezo_calibration_export_info_t *info_out) {
  char buffer[EZO_CALIBRATION_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result = ezo_calibration_read_uart_data_then_ok(device,
                                                               buffer,
                                                               sizeof(buffer),
                                                               &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_calibration_parse_export_info(buffer, response_len, info_out);
}

ezo_result_t ezo_calibration_read_export_chunk_uart(
    ezo_uart_device_t *device,
    char *buffer,
    size_t buffer_len,
    size_t *chunk_len_out,
    ezo_uart_response_kind_t *response_kind_out) {
  if (buffer == NULL || buffer_len == 0 || chunk_len_out == NULL || response_kind_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  return ezo_uart_read_line(device, buffer, buffer_len, chunk_len_out, response_kind_out);
}

ezo_result_t ezo_calibration_read_import_result_uart(
    ezo_uart_device_t *device,
    ezo_uart_response_kind_t *response_kind_out) {
  char buffer[8];
  size_t response_len = 0;
  ezo_result_t result = EZO_OK;

  if (response_kind_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  result = ezo_uart_read_line(device,
                              buffer,
                              sizeof(buffer),
                              &response_len,
                              response_kind_out);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_uart_response_kind_is_control(*response_kind_out) ? EZO_OK : EZO_ERR_PROTOCOL;
}
