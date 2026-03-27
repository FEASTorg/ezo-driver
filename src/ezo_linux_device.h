#ifndef EZO_LINUX_DEVICE_H
#define EZO_LINUX_DEVICE_H

#include "ezo_i2c_linux_i2c.h"
#include "ezo_uart_posix_serial.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  ezo_i2c_device_t core;
  ezo_linux_i2c_context_t context;
  int owns_fd;
} ezo_linux_i2c_device_t;

typedef struct {
  ezo_uart_device_t core;
  ezo_uart_posix_serial_t serial;
} ezo_linux_uart_device_t;

ezo_result_t ezo_linux_i2c_device_open_bus(ezo_linux_i2c_device_t *device,
                                           uint32_t bus_index,
                                           uint8_t address);

ezo_result_t ezo_linux_i2c_device_open_path(ezo_linux_i2c_device_t *device,
                                            const char *path,
                                            uint8_t address);

void ezo_linux_i2c_device_close(ezo_linux_i2c_device_t *device);

ezo_i2c_device_t *ezo_linux_i2c_device_core(ezo_linux_i2c_device_t *device);

ezo_result_t ezo_linux_uart_device_open(ezo_linux_uart_device_t *device,
                                        const char *path,
                                        uint32_t baud_rate,
                                        uint32_t read_timeout_ms);

void ezo_linux_uart_device_close(ezo_linux_uart_device_t *device);

ezo_uart_device_t *ezo_linux_uart_device_core(ezo_linux_uart_device_t *device);

#ifdef __cplusplus
}
#endif

#endif
