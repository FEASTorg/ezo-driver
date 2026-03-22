/*
Purpose: inspect a UART device and print repo metadata plus shared control state.
Defaults: /dev/ttyUSB0 at 9600 baud.
Assumptions: the device is in UART mode or still at its shipping UART default.
Next: read readiness_check.c for product-specific setup state.
*/

#include "example_base.h"
#include "example_uart.h"

#include "ezo_control.h"
#include "ezo_product.h"

#include <stdio.h>

#define UART_QUERY(send_expr, read_expr) \
  do {                                   \
    result = (send_expr);                \
    if (result != EZO_OK) {              \
      return result;                     \
    }                                    \
    ezo_example_wait_hint(&hint);        \
    result = (read_expr);                \
    if (result != EZO_OK) {              \
      return result;                     \
    }                                    \
  } while (0)

static const char *support_name(ezo_product_support_t support) {
  switch (support) {
    case EZO_PRODUCT_SUPPORT_METADATA:
      return "metadata";
    case EZO_PRODUCT_SUPPORT_TYPED_READ:
      return "typed_read";
    case EZO_PRODUCT_SUPPORT_FULL:
      return "full";
    default:
      return "unknown";
  }
}

static const char *transport_name(ezo_product_transport_t transport) {
  switch (transport) {
    case EZO_PRODUCT_TRANSPORT_UART:
      return "uart";
    case EZO_PRODUCT_TRANSPORT_I2C:
      return "i2c";
    default:
      return "unknown";
  }
}

static const char *default_state_name(ezo_product_default_state_t state) {
  switch (state) {
    case EZO_PRODUCT_DEFAULT_DISABLED:
      return "disabled";
    case EZO_PRODUCT_DEFAULT_ENABLED:
      return "enabled";
    case EZO_PRODUCT_DEFAULT_QUERY_REQUIRED:
      return "query_required";
    default:
      return "unknown";
  }
}

static const char *output_schema_name(ezo_product_output_schema_t schema) {
  switch (schema) {
    case EZO_PRODUCT_OUTPUT_SCHEMA_SCALAR_SINGLE:
      return "scalar_single";
    case EZO_PRODUCT_OUTPUT_SCHEMA_PRIMARY_ONLY:
      return "primary_only";
    case EZO_PRODUCT_OUTPUT_SCHEMA_QUERY_REQUIRED:
      return "query_required";
    default:
      return "unknown";
  }
}

static void print_device_info(const ezo_device_info_t *info) {
  if (info == NULL) {
    return;
  }

  printf("product_id=%d\n", (int)info->product_id);
  printf("product_code=%s\n", info->product_code);
  printf("firmware_version=%s\n", info->firmware_version);
}

static void print_product_metadata(const ezo_product_metadata_t *metadata) {
  if (metadata == NULL) {
    printf("metadata_known=0\n");
    return;
  }

  printf("metadata_known=1\n");
  printf("family_name=%s\n", metadata->family_name);
  printf("vendor_short_code=%s\n", metadata->vendor_short_code);
  printf("support_tier=%s\n", support_name(metadata->support_tier));
  printf("default_transport=%s\n", transport_name(metadata->default_transport));
  printf("default_i2c_address=%u\n", (unsigned)metadata->default_i2c_address);
  printf("default_continuous_mode=%s\n",
         default_state_name(metadata->default_continuous_mode));
  printf("default_response_codes=%s\n",
         default_state_name(metadata->default_response_codes));
  printf("default_output_schema=%s\n",
         output_schema_name(metadata->default_output_schema));
  printf("default_output_count=%u\n", (unsigned)metadata->default_output_count);
  printf("capability_flags=%u\n", (unsigned)metadata->capability_flags);
  printf("command_family_flags=%u\n", (unsigned)metadata->command_family_flags);
  printf("uart_timing_generic_ms=%u\n", (unsigned)metadata->uart_timing.generic_ms);
  printf("uart_timing_read_ms=%u\n", (unsigned)metadata->uart_timing.read_ms);
  printf("uart_timing_read_with_temp_comp_ms=%u\n",
         (unsigned)metadata->uart_timing.read_with_temp_comp_ms);
  printf("uart_timing_calibration_ms=%u\n", (unsigned)metadata->uart_timing.calibration_ms);
  printf("i2c_timing_generic_ms=%u\n", (unsigned)metadata->i2c_timing.generic_ms);
  printf("i2c_timing_read_ms=%u\n", (unsigned)metadata->i2c_timing.read_ms);
  printf("i2c_timing_read_with_temp_comp_ms=%u\n",
         (unsigned)metadata->i2c_timing.read_with_temp_comp_ms);
  printf("i2c_timing_calibration_ms=%u\n", (unsigned)metadata->i2c_timing.calibration_ms);
}

static ezo_result_t query_info_uart(ezo_uart_device_t *device, ezo_device_info_t *info_out) {
  ezo_timing_hint_t hint;
  ezo_result_t result = EZO_OK;

  if (device == NULL || info_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  result = ezo_control_send_info_query_uart(device, EZO_PRODUCT_UNKNOWN, &hint);
  if (result != EZO_OK) {
    return result;
  }

  ezo_example_wait_hint(&hint);
  return ezo_control_read_info_uart(device, info_out);
}

static ezo_result_t print_shared_control_uart(ezo_uart_device_t *device,
                                              ezo_product_id_t product_id) {
  ezo_timing_hint_t hint;
  ezo_control_name_t name;
  ezo_control_status_t status;
  ezo_control_led_status_t led;
  ezo_control_protocol_lock_status_t protocol_lock;
  ezo_control_baud_status_t baud;
  ezo_control_response_code_status_t response_code;
  ezo_result_t result = EZO_OK;

  if (device == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  UART_QUERY(ezo_control_send_name_query_uart(device, product_id, &hint),
             ezo_control_read_name_uart(device, &name));
  UART_QUERY(ezo_control_send_status_query_uart(device, product_id, &hint),
             ezo_control_read_status_uart(device, &status));
  UART_QUERY(ezo_control_send_led_query_uart(device, product_id, &hint),
             ezo_control_read_led_uart(device, &led));
  UART_QUERY(ezo_control_send_protocol_lock_query_uart(device, product_id, &hint),
             ezo_control_read_protocol_lock_uart(device, &protocol_lock));
  UART_QUERY(ezo_control_send_baud_query_uart(device, product_id, &hint),
             ezo_control_read_baud_uart(device, &baud));
  UART_QUERY(ezo_control_send_response_code_query_uart(device, product_id, &hint),
             ezo_control_read_response_code_uart(device, &response_code));

  printf("device_name=%s\n", name.name);
  printf("restart_code=%c\n", status.restart_code);
  printf("supply_voltage_v=%.3f\n", status.supply_voltage);
  printf("led_enabled=%u\n", (unsigned)led.enabled);
  printf("protocol_lock_enabled=%u\n", (unsigned)protocol_lock.enabled);
  printf("baud_rate=%u\n", (unsigned)baud.baud_rate);
  printf("response_codes_enabled=%u\n", (unsigned)response_code.enabled);
  return EZO_OK;
}

int main(int argc, char **argv) {
  ezo_example_uart_options_t options;
  ezo_example_uart_session_t session;
  ezo_device_info_t info;
  const ezo_product_metadata_t *metadata = NULL;
  uint8_t response_codes_before = 0;
  uint8_t response_codes_after = 0;
  ezo_result_t result = EZO_OK;
  int next_arg = 0;

  if (!ezo_example_parse_uart_options(argc,
                                      argv,
                                      EZO_EXAMPLE_UART_DEFAULT_BAUD_RATE,
                                      &options,
                                      &next_arg)) {
    fprintf(stderr, "usage: %s [device_path] [baud]\n", argv[0]);
    return 1;
  }

  result = ezo_example_open_uart(options.device_path, options.baud, &session);
  if (result != EZO_OK) {
    return ezo_example_print_error("open_uart", result);
  }

  result = ezo_example_uart_bootstrap_response_codes(&session.device,
                                                     EZO_PRODUCT_UNKNOWN,
                                                     &response_codes_before,
                                                     &response_codes_after);
  if (result != EZO_OK) {
    ezo_example_close_uart(&session);
    return ezo_example_print_error("bootstrap_response_codes", result);
  }

  result = query_info_uart(&session.device, &info);
  if (result != EZO_OK) {
    ezo_example_close_uart(&session);
    return ezo_example_print_error("query_info", result);
  }

  metadata = ezo_product_get_metadata(info.product_id);

  printf("transport=uart\n");
  printf("device_path=%s\n", options.device_path);
  printf("baud_rate=%u\n", (unsigned)options.baud_rate);
  printf("response_codes_before_bootstrap=%u\n", (unsigned)response_codes_before);
  printf("response_codes_after_bootstrap=%u\n", (unsigned)response_codes_after);
  print_device_info(&info);
  print_product_metadata(metadata);

  result = print_shared_control_uart(&session.device, info.product_id);
  ezo_example_close_uart(&session);
  if (result != EZO_OK) {
    return ezo_example_print_error("shared_control", result);
  }

  return 0;
}
