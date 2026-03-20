#include "ezo.h"
#include "ezo_common.h"

ezo_result_t ezo_get_timing_hint_for_command_kind(ezo_command_kind_t kind,
                                                  ezo_timing_hint_t *timing_hint) {
  uint32_t wait_ms = 0;

  if (timing_hint == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  switch (kind) {
  case EZO_COMMAND_GENERIC:
    wait_ms = 300;
    break;
  case EZO_COMMAND_READ:
  case EZO_COMMAND_READ_WITH_TEMP_COMP:
    wait_ms = 1000;
    break;
  case EZO_COMMAND_CALIBRATION:
    wait_ms = 1200;
    break;
  default:
    return EZO_ERR_INVALID_ARGUMENT;
  }

  timing_hint->wait_ms = wait_ms;
  return EZO_OK;
}

ezo_result_t ezo_parse_double(const char *buffer, size_t buffer_len, double *value_out) {
  return ezo_common_parse_double(buffer, buffer_len, value_out);
}
