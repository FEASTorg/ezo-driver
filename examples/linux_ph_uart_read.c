#include "ezo_control.h"
#include "ezo_ph.h"
#include "ezo_uart_posix_serial.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int parse_baud(const char *text, ezo_uart_posix_baud_t *baud_out) {
  if (text == NULL || baud_out == NULL) {
    return 0;
  }

  if (strcmp(text, "300") == 0) {
    *baud_out = EZO_UART_POSIX_BAUD_300;
    return 1;
  }
  if (strcmp(text, "1200") == 0) {
    *baud_out = EZO_UART_POSIX_BAUD_1200;
    return 1;
  }
  if (strcmp(text, "2400") == 0) {
    *baud_out = EZO_UART_POSIX_BAUD_2400;
    return 1;
  }
  if (strcmp(text, "9600") == 0) {
    *baud_out = EZO_UART_POSIX_BAUD_9600;
    return 1;
  }
  if (strcmp(text, "19200") == 0) {
    *baud_out = EZO_UART_POSIX_BAUD_19200;
    return 1;
  }
  if (strcmp(text, "38400") == 0) {
    *baud_out = EZO_UART_POSIX_BAUD_38400;
    return 1;
  }
  if (strcmp(text, "57600") == 0) {
    *baud_out = EZO_UART_POSIX_BAUD_57600;
    return 1;
  }
  if (strcmp(text, "115200") == 0) {
    *baud_out = EZO_UART_POSIX_BAUD_115200;
    return 1;
  }

  return 0;
}

static ezo_result_t ensure_response_codes_enabled(ezo_uart_device_t *device) {
  ezo_timing_hint_t hint;
  ezo_control_response_code_status_t response_code;
  ezo_result_t result =
      ezo_control_send_response_code_query_uart(device, EZO_PRODUCT_PH, &hint);
  if (result != EZO_OK) {
    return result;
  }

  usleep((useconds_t)(hint.wait_ms * 1000U));

  result = ezo_control_read_response_code_uart(device, &response_code);
  if (result != EZO_OK || response_code.enabled != 0) {
    return result;
  }

  result = ezo_control_send_response_code_set_uart(device, EZO_PRODUCT_PH, 1, &hint);
  if (result != EZO_OK) {
    return result;
  }

  usleep((useconds_t)(hint.wait_ms * 1000U));
  return ezo_uart_read_ok(device);
}

int main(int argc, char **argv) {
  const char *device_path = "/dev/ttyUSB0";
  ezo_uart_posix_baud_t baud = EZO_UART_POSIX_BAUD_9600;
  ezo_uart_posix_serial_t serial;
  ezo_uart_device_t device;
  ezo_timing_hint_t hint;
  ezo_ph_reading_t reading;
  ezo_result_t result = EZO_OK;

  if (argc > 1) {
    device_path = argv[1];
  }

  if (argc > 2 && !parse_baud(argv[2], &baud)) {
    fprintf(stderr, "unsupported baud: %s\n", argv[2]);
    return 1;
  }

  result = ezo_uart_posix_serial_open(&serial, device_path, baud, 500);
  if (result != EZO_OK) {
    fprintf(stderr, "open failed: %d\n", (int)result);
    return 1;
  }

  result = ezo_uart_device_init(&device, ezo_uart_posix_serial_transport(), &serial);
  if (result != EZO_OK) {
    ezo_uart_posix_serial_close(&serial);
    return 1;
  }

  result = ezo_uart_discard_input(&device);
  if (result != EZO_OK) {
    ezo_uart_posix_serial_close(&serial);
    return 1;
  }

  result = ensure_response_codes_enabled(&device);
  if (result != EZO_OK) {
    fprintf(stderr, "response-code bootstrap failed: %d\n", (int)result);
    ezo_uart_posix_serial_close(&serial);
    return 1;
  }

  result = ezo_ph_send_read_uart(&device, &hint);
  if (result != EZO_OK) {
    ezo_uart_posix_serial_close(&serial);
    return 1;
  }

  usleep((useconds_t)(hint.wait_ms * 1000U));

  result = ezo_ph_read_response_uart(&device, &reading);
  ezo_uart_posix_serial_close(&serial);
  if (result != EZO_OK) {
    fprintf(stderr, "typed UART pH read failed: %d\n", (int)result);
    return 1;
  }

  printf("pH=%.3f\n", reading.ph);
  return 0;
}
