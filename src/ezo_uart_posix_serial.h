#ifndef EZO_UART_POSIX_SERIAL_H
#define EZO_UART_POSIX_SERIAL_H

#include "ezo_uart.h"

#include <stdint.h>
#include <termios.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  EZO_UART_POSIX_BAUD_300 = 0,
  EZO_UART_POSIX_BAUD_1200,
  EZO_UART_POSIX_BAUD_2400,
  EZO_UART_POSIX_BAUD_9600,
  EZO_UART_POSIX_BAUD_19200,
  EZO_UART_POSIX_BAUD_38400,
  EZO_UART_POSIX_BAUD_57600,
  EZO_UART_POSIX_BAUD_115200
} ezo_uart_posix_baud_t;

typedef struct {
  int fd;
  uint32_t read_timeout_ms;
  int owns_fd;
  int has_saved_termios;
  struct termios saved_termios;
} ezo_uart_posix_serial_t;

ezo_result_t ezo_uart_posix_serial_open(ezo_uart_posix_serial_t *serial,
                                        const char *path,
                                        ezo_uart_posix_baud_t baud,
                                        uint32_t read_timeout_ms);

void ezo_uart_posix_serial_close(ezo_uart_posix_serial_t *serial);

const ezo_uart_transport_t *ezo_uart_posix_serial_transport(void);

#ifdef __cplusplus
}
#endif

#endif
