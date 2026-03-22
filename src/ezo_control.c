#include "ezo_control.h"

#include "ezo_common.h"
#include "ezo_parse.h"

#include <string.h>

enum {
  EZO_CONTROL_RESPONSE_BUFFER_LEN = 96,
  EZO_CONTROL_VENDOR_NAME_MAX_LEN = 16
};

static ezo_result_t ezo_control_copy_command(char *buffer,
                                             size_t buffer_len,
                                             const char *command) {
  size_t command_len = 0;

  if (buffer == NULL || buffer_len == 0 || command == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  command_len = strlen(command);
  if (command_len + 1U > buffer_len) {
    return EZO_ERR_BUFFER_TOO_SMALL;
  }

  memcpy(buffer, command, command_len + 1U);
  return EZO_OK;
}

static ezo_result_t ezo_control_copy_span_string(ezo_text_span_t span,
                                                 char *buffer,
                                                 size_t buffer_len) {
  if (buffer == NULL || buffer_len == 0) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  if (span.text == NULL) {
    return EZO_ERR_PARSE;
  }

  if (span.length + 1U > buffer_len) {
    return EZO_ERR_BUFFER_TOO_SMALL;
  }

  if (span.length > 0) {
    memcpy(buffer, span.text, span.length);
  }
  buffer[span.length] = '\0';
  return EZO_OK;
}

static int ezo_control_name_char_is_valid(char value) {
  return value >= '!' && value <= '~';
}

static ezo_result_t ezo_control_parse_query_bool(const char *buffer,
                                                 size_t buffer_len,
                                                 const char *prefix,
                                                 uint8_t *enabled_out) {
  ezo_text_span_t fields[1];
  size_t field_count = 0;
  uint32_t value = 0;
  ezo_result_t result = EZO_OK;

  if (prefix == NULL || enabled_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  result = ezo_parse_prefixed_fields(buffer, buffer_len, prefix, fields, 1, &field_count);
  if (result != EZO_OK) {
    return result;
  }

  if (field_count != 1) {
    return EZO_ERR_PARSE;
  }

  result = ezo_parse_text_span_uint32(fields[0], &value);
  if (result != EZO_OK || value > 1U) {
    return EZO_ERR_PARSE;
  }

  *enabled_out = (uint8_t)value;
  return EZO_OK;
}

static ezo_result_t ezo_control_send_i2c_command(ezo_i2c_device_t *device,
                                                 ezo_product_id_t product_id,
                                                 const char *command,
                                                 ezo_command_kind_t kind,
                                                 ezo_timing_hint_t *timing_hint) {
  ezo_timing_hint_t local_hint;
  ezo_result_t result =
      ezo_product_resolve_timing_hint(product_id,
                                      EZO_PRODUCT_TRANSPORT_I2C,
                                      kind,
                                      timing_hint != NULL ? timing_hint : &local_hint);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_send_command(device, command, kind, NULL);
}

static ezo_result_t ezo_control_send_uart_command(ezo_uart_device_t *device,
                                                  ezo_product_id_t product_id,
                                                  const char *command,
                                                  ezo_command_kind_t kind,
                                                  ezo_timing_hint_t *timing_hint) {
  ezo_timing_hint_t local_hint;
  ezo_result_t result =
      ezo_product_resolve_timing_hint(product_id,
                                      EZO_PRODUCT_TRANSPORT_UART,
                                      kind,
                                      timing_hint != NULL ? timing_hint : &local_hint);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_uart_send_command(device, command, kind, NULL);
}

static ezo_result_t ezo_control_read_i2c_text(ezo_i2c_device_t *device,
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

static ezo_result_t ezo_control_read_uart_line_of_kind(ezo_uart_device_t *device,
                                                       ezo_uart_response_kind_t expected_kind,
                                                       char *buffer,
                                                       size_t buffer_len,
                                                       size_t *response_len) {
  ezo_uart_response_kind_t kind = EZO_UART_RESPONSE_UNKNOWN;
  ezo_result_t result =
      ezo_uart_read_line(device, buffer, buffer_len, response_len, &kind);
  if (result != EZO_OK) {
    return result;
  }

  if (kind != expected_kind) {
    return EZO_ERR_PROTOCOL;
  }

  return EZO_OK;
}

static ezo_result_t ezo_control_read_uart_data_then_ok(ezo_uart_device_t *device,
                                                       char *buffer,
                                                       size_t buffer_len,
                                                       size_t *response_len) {
  ezo_result_t result = ezo_control_read_uart_line_of_kind(device,
                                                           EZO_UART_RESPONSE_DATA,
                                                           buffer,
                                                           buffer_len,
                                                           response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_uart_read_ok(device);
}

ezo_result_t ezo_control_parse_name(const char *buffer,
                                    size_t buffer_len,
                                    ezo_control_name_t *name_out) {
  ezo_text_span_t fields[1];
  size_t field_count = 0;
  ezo_result_t result = EZO_OK;

  if (name_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  result = ezo_parse_prefixed_fields(buffer, buffer_len, "?Name", fields, 1, &field_count);
  if (result != EZO_OK) {
    return result;
  }

  if (field_count != 1) {
    return EZO_ERR_PARSE;
  }

  return ezo_control_copy_span_string(fields[0], name_out->name, sizeof(name_out->name));
}

ezo_result_t ezo_control_parse_status(const char *buffer,
                                      size_t buffer_len,
                                      ezo_control_status_t *status_out) {
  ezo_text_span_t fields[2];
  size_t field_count = 0;
  ezo_result_t result = EZO_OK;

  if (status_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  result = ezo_parse_prefixed_fields(buffer, buffer_len, "?Status", fields, 2, &field_count);
  if (result != EZO_OK) {
    return result;
  }

  if (field_count != 2 || fields[0].text == NULL || fields[0].length != 1) {
    return EZO_ERR_PARSE;
  }

  status_out->restart_code = fields[0].text[0];
  return ezo_parse_text_span_double(fields[1], &status_out->supply_voltage);
}

ezo_result_t ezo_control_parse_led(const char *buffer,
                                   size_t buffer_len,
                                   ezo_control_led_status_t *status_out) {
  if (status_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  return ezo_control_parse_query_bool(buffer, buffer_len, "?L", &status_out->enabled);
}

ezo_result_t ezo_control_parse_protocol_lock(
    const char *buffer,
    size_t buffer_len,
    ezo_control_protocol_lock_status_t *status_out) {
  if (status_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  return ezo_control_parse_query_bool(buffer, buffer_len, "?Plock", &status_out->enabled);
}

ezo_result_t ezo_control_parse_baud(const char *buffer,
                                    size_t buffer_len,
                                    ezo_control_baud_status_t *status_out) {
  ezo_text_span_t fields[1];
  size_t field_count = 0;
  ezo_result_t result = EZO_OK;

  if (status_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  result = ezo_parse_prefixed_fields(buffer, buffer_len, "?Baud", fields, 1, &field_count);
  if (result != EZO_OK) {
    return result;
  }

  if (field_count != 1) {
    return EZO_ERR_PARSE;
  }

  return ezo_parse_text_span_uint32(fields[0], &status_out->baud_rate);
}

ezo_result_t ezo_control_parse_response_code(
    const char *buffer,
    size_t buffer_len,
    ezo_control_response_code_status_t *status_out) {
  if (status_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  return ezo_control_parse_query_bool(buffer, buffer_len, "?*OK", &status_out->enabled);
}

ezo_result_t ezo_control_build_name_command(char *buffer,
                                            size_t buffer_len,
                                            const char *name) {
  size_t prefix_len = strlen("Name,");
  size_t name_len = 0;
  size_t i = 0;

  if (buffer == NULL || buffer_len == 0 || name == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  name_len = strlen(name);
  if (name_len > EZO_CONTROL_VENDOR_NAME_MAX_LEN) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  if (prefix_len + name_len + 1U > buffer_len) {
    return EZO_ERR_BUFFER_TOO_SMALL;
  }

  for (i = 0; i < name_len; ++i) {
    if (!ezo_control_name_char_is_valid(name[i])) {
      return EZO_ERR_INVALID_ARGUMENT;
    }
  }

  memcpy(buffer, "Name,", prefix_len);
  if (name_len > 0) {
    memcpy(buffer + prefix_len, name, name_len);
  }
  buffer[prefix_len + name_len] = '\0';
  return EZO_OK;
}

ezo_result_t ezo_control_build_led_command(char *buffer,
                                           size_t buffer_len,
                                           uint8_t enabled) {
  return ezo_control_copy_command(buffer, buffer_len, enabled != 0 ? "L,1" : "L,0");
}

ezo_result_t ezo_control_build_protocol_lock_command(char *buffer,
                                                     size_t buffer_len,
                                                     uint8_t enabled) {
  return ezo_control_copy_command(buffer, buffer_len, enabled != 0 ? "Plock,1"
                                                                    : "Plock,0");
}

ezo_result_t ezo_control_build_switch_to_i2c_command(char *buffer,
                                                     size_t buffer_len,
                                                     uint8_t i2c_address) {
  if (i2c_address == 0 || i2c_address > 127U) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  return ezo_common_format_fixed_command(buffer, buffer_len, "I2C,", (double)i2c_address, 0);
}

ezo_result_t ezo_control_build_switch_to_uart_command(char *buffer,
                                                      size_t buffer_len,
                                                      uint32_t baud_rate) {
  if (baud_rate == 0U) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  return ezo_common_format_fixed_command(buffer, buffer_len, "Baud,", (double)baud_rate, 0);
}

ezo_result_t ezo_control_build_response_code_command(char *buffer,
                                                     size_t buffer_len,
                                                     uint8_t enabled) {
  return ezo_control_copy_command(buffer, buffer_len, enabled != 0 ? "*OK,1" : "*OK,0");
}

ezo_result_t ezo_control_send_info_query_i2c(ezo_i2c_device_t *device,
                                             ezo_product_id_t product_id,
                                             ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_i2c_command(device, product_id, "i", EZO_COMMAND_GENERIC, timing_hint);
}

ezo_result_t ezo_control_send_name_query_i2c(ezo_i2c_device_t *device,
                                             ezo_product_id_t product_id,
                                             ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_i2c_command(device, product_id, "Name,?", EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_name_set_i2c(ezo_i2c_device_t *device,
                                           ezo_product_id_t product_id,
                                           const char *name,
                                           ezo_timing_hint_t *timing_hint) {
  char command[48];
  ezo_result_t result = ezo_control_build_name_command(command, sizeof(command), name);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_i2c_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_status_query_i2c(ezo_i2c_device_t *device,
                                               ezo_product_id_t product_id,
                                               ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_i2c_command(device, product_id, "Status", EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_led_query_i2c(ezo_i2c_device_t *device,
                                            ezo_product_id_t product_id,
                                            ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_i2c_command(device, product_id, "L,?", EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_led_set_i2c(ezo_i2c_device_t *device,
                                          ezo_product_id_t product_id,
                                          uint8_t enabled,
                                          ezo_timing_hint_t *timing_hint) {
  char command[8];
  ezo_result_t result = ezo_control_build_led_command(command, sizeof(command), enabled);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_i2c_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_find_i2c(ezo_i2c_device_t *device,
                                       ezo_product_id_t product_id,
                                       ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_i2c_command(device, product_id, "Find", EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_sleep_i2c(ezo_i2c_device_t *device,
                                        ezo_product_id_t product_id,
                                        ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_i2c_command(device, product_id, "Sleep", EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_factory_reset_i2c(ezo_i2c_device_t *device,
                                                ezo_product_id_t product_id,
                                                ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_i2c_command(device, product_id, "Factory", EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_protocol_lock_query_i2c(
    ezo_i2c_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_i2c_command(device, product_id, "Plock,?", EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_protocol_lock_set_i2c(
    ezo_i2c_device_t *device,
    ezo_product_id_t product_id,
    uint8_t enabled,
    ezo_timing_hint_t *timing_hint) {
  char command[16];
  ezo_result_t result =
      ezo_control_build_protocol_lock_command(command, sizeof(command), enabled);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_i2c_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_send_switch_to_uart_i2c(ezo_i2c_device_t *device,
                                                 ezo_product_id_t product_id,
                                                 uint32_t baud_rate,
                                                 ezo_timing_hint_t *timing_hint) {
  char command[24];
  ezo_result_t result =
      ezo_control_build_switch_to_uart_command(command, sizeof(command), baud_rate);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_i2c_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                      timing_hint);
}

ezo_result_t ezo_control_read_info_i2c(ezo_i2c_device_t *device,
                                       ezo_device_info_t *info_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result = ezo_control_read_i2c_text(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_parse_device_info(buffer, response_len, info_out);
}

ezo_result_t ezo_control_read_name_i2c(ezo_i2c_device_t *device,
                                       ezo_control_name_t *name_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result = ezo_control_read_i2c_text(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_name(buffer, response_len, name_out);
}

ezo_result_t ezo_control_read_status_i2c(ezo_i2c_device_t *device,
                                         ezo_control_status_t *status_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result = ezo_control_read_i2c_text(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_status(buffer, response_len, status_out);
}

ezo_result_t ezo_control_read_led_i2c(ezo_i2c_device_t *device,
                                      ezo_control_led_status_t *status_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result = ezo_control_read_i2c_text(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_led(buffer, response_len, status_out);
}

ezo_result_t ezo_control_read_protocol_lock_i2c(
    ezo_i2c_device_t *device,
    ezo_control_protocol_lock_status_t *status_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result = ezo_control_read_i2c_text(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_protocol_lock(buffer, response_len, status_out);
}

ezo_result_t ezo_control_send_info_query_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id,
                                              ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "i", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_name_query_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id,
                                              ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "Name,?", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_name_set_uart(ezo_uart_device_t *device,
                                            ezo_product_id_t product_id,
                                            const char *name,
                                            ezo_timing_hint_t *timing_hint) {
  char command[48];
  ezo_result_t result = ezo_control_build_name_command(command, sizeof(command), name);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_uart_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_status_query_uart(ezo_uart_device_t *device,
                                                ezo_product_id_t product_id,
                                                ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "Status", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_led_query_uart(ezo_uart_device_t *device,
                                             ezo_product_id_t product_id,
                                             ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "L,?", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_led_set_uart(ezo_uart_device_t *device,
                                           ezo_product_id_t product_id,
                                           uint8_t enabled,
                                           ezo_timing_hint_t *timing_hint) {
  char command[8];
  ezo_result_t result = ezo_control_build_led_command(command, sizeof(command), enabled);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_uart_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_find_uart(ezo_uart_device_t *device,
                                        ezo_product_id_t product_id,
                                        ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "Find", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_sleep_uart(ezo_uart_device_t *device,
                                         ezo_product_id_t product_id,
                                         ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "Sleep", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_factory_reset_uart(ezo_uart_device_t *device,
                                                 ezo_product_id_t product_id,
                                                 ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "Factory", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_protocol_lock_query_uart(
    ezo_uart_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "Plock,?", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_protocol_lock_set_uart(
    ezo_uart_device_t *device,
    ezo_product_id_t product_id,
    uint8_t enabled,
    ezo_timing_hint_t *timing_hint) {
  char command[16];
  ezo_result_t result =
      ezo_control_build_protocol_lock_command(command, sizeof(command), enabled);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_uart_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_baud_query_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id,
                                              ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "Baud,?", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_response_code_query_uart(
    ezo_uart_device_t *device,
    ezo_product_id_t product_id,
    ezo_timing_hint_t *timing_hint) {
  return ezo_control_send_uart_command(device, product_id, "*OK,?", EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_response_code_set_uart(ezo_uart_device_t *device,
                                                     ezo_product_id_t product_id,
                                                     uint8_t enabled,
                                                     ezo_timing_hint_t *timing_hint) {
  char command[8];
  ezo_result_t result =
      ezo_control_build_response_code_command(command, sizeof(command), enabled);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_uart_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_switch_to_i2c_uart(ezo_uart_device_t *device,
                                                 ezo_product_id_t product_id,
                                                 uint8_t i2c_address,
                                                 ezo_timing_hint_t *timing_hint) {
  char command[16];
  ezo_result_t result =
      ezo_control_build_switch_to_i2c_command(command, sizeof(command), i2c_address);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_uart_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_send_switch_to_uart_uart(ezo_uart_device_t *device,
                                                  ezo_product_id_t product_id,
                                                  uint32_t baud_rate,
                                                  ezo_timing_hint_t *timing_hint) {
  char command[24];
  ezo_result_t result =
      ezo_control_build_switch_to_uart_command(command, sizeof(command), baud_rate);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_send_uart_command(device, product_id, command, EZO_COMMAND_GENERIC,
                                       timing_hint);
}

ezo_result_t ezo_control_read_info_uart(ezo_uart_device_t *device,
                                        ezo_device_info_t *info_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result =
      ezo_control_read_uart_data_then_ok(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_parse_device_info(buffer, response_len, info_out);
}

ezo_result_t ezo_control_read_name_uart(ezo_uart_device_t *device,
                                        ezo_control_name_t *name_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result =
      ezo_control_read_uart_data_then_ok(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_name(buffer, response_len, name_out);
}

ezo_result_t ezo_control_read_status_uart(ezo_uart_device_t *device,
                                          ezo_control_status_t *status_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result =
      ezo_control_read_uart_data_then_ok(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_status(buffer, response_len, status_out);
}

ezo_result_t ezo_control_read_led_uart(ezo_uart_device_t *device,
                                       ezo_control_led_status_t *status_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result =
      ezo_control_read_uart_data_then_ok(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_led(buffer, response_len, status_out);
}

ezo_result_t ezo_control_read_protocol_lock_uart(
    ezo_uart_device_t *device,
    ezo_control_protocol_lock_status_t *status_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result =
      ezo_control_read_uart_data_then_ok(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_protocol_lock(buffer, response_len, status_out);
}

ezo_result_t ezo_control_read_baud_uart(ezo_uart_device_t *device,
                                        ezo_control_baud_status_t *status_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result =
      ezo_control_read_uart_data_then_ok(device, buffer, sizeof(buffer), &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_baud(buffer, response_len, status_out);
}

ezo_result_t ezo_control_read_response_code_uart(
    ezo_uart_device_t *device,
    ezo_control_response_code_status_t *status_out) {
  char buffer[EZO_CONTROL_RESPONSE_BUFFER_LEN];
  size_t response_len = 0;
  ezo_result_t result = ezo_control_read_uart_line_of_kind(device,
                                                           EZO_UART_RESPONSE_DATA,
                                                           buffer,
                                                           sizeof(buffer),
                                                           &response_len);
  if (result != EZO_OK) {
    return result;
  }

  return ezo_control_parse_response_code(buffer, response_len, status_out);
}
