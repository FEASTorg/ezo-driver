/*
Purpose: inspect an I2C device and print repo metadata plus shared control state.
Defaults: /dev/i2c-1 and address 99.
Assumptions: the device is already in I2C mode and responds at the selected address.
Next: read readiness_check.c to inspect product-specific setup state.
*/

#include "example_base.h"
#include "example_control.h"
#include "example_i2c.h"

#include "ezo_product.h"

#include <stdio.h>

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

int main(int argc, char **argv) {
  ezo_example_i2c_options_t options;
  ezo_example_i2c_session_t session;
  ezo_device_info_t info;
  const ezo_product_metadata_t *metadata = NULL;
  ezo_result_t result = EZO_OK;
  int next_arg = 0;

  if (!ezo_example_parse_i2c_options(argc, argv, 99U, &options, &next_arg)) {
    fprintf(stderr, "usage: %s [device_path] [address]\n", argv[0]);
    return 1;
  }

  result = ezo_example_open_i2c(options.device_path, options.address, &session);
  if (result != EZO_OK) {
    return ezo_example_print_error("open_i2c", result);
  }

  result = ezo_example_query_info_i2c(&session.device, &info);
  if (result != EZO_OK) {
    ezo_example_close_i2c(&session);
    return ezo_example_print_error("query_info", result);
  }

  metadata = ezo_product_get_metadata(info.product_id);

  printf("transport=i2c\n");
  printf("device_path=%s\n", options.device_path);
  printf("address=%u\n", (unsigned)options.address);
  print_device_info(&info);
  print_product_metadata(metadata);

  result = ezo_example_print_shared_control_i2c(&session.device, info.product_id);
  ezo_example_close_i2c(&session);
  if (result != EZO_OK) {
    return ezo_example_print_error("shared_control", result);
  }

  return 0;
}
