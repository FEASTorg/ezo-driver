#include "ezo_uart.h"
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

int main(int argc, char **argv) {
  const char *device_path = "/dev/ttyUSB0";
  ezo_uart_posix_baud_t baud = EZO_UART_POSIX_BAUD_9600;
  ezo_uart_posix_serial_t serial;
  ezo_uart_device_t device;
  ezo_timing_hint_t hint;
  ezo_uart_response_kind_t kind = EZO_UART_RESPONSE_UNKNOWN;
  char response[64];
  size_t response_len = 0;
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

  result = ezo_uart_send_command(&device, "i", EZO_COMMAND_GENERIC, &hint);
  if (result != EZO_OK) {
    ezo_uart_posix_serial_close(&serial);
    return 1;
  }

  usleep((useconds_t)(hint.wait_ms * 1000U));

  result = ezo_uart_read_response(&device, response, sizeof(response), &response_len, &kind);
  ezo_uart_posix_serial_close(&serial);

  if (result != EZO_OK) {
    fprintf(stderr, "read failed: %d\n", (int)result);
    return 1;
  }

  printf("kind=%d response=%s\n", (int)kind, response);
  return 0;
}
