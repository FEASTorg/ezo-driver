#include "ezo_linux_device.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static ezo_result_t ezo_linux_uart_map_baud_rate(uint32_t baud_rate,
                                                 ezo_uart_posix_baud_t *baud_out) {
  if (baud_out == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  switch (baud_rate) {
  case 300:
    *baud_out = EZO_UART_POSIX_BAUD_300;
    return EZO_OK;
  case 1200:
    *baud_out = EZO_UART_POSIX_BAUD_1200;
    return EZO_OK;
  case 2400:
    *baud_out = EZO_UART_POSIX_BAUD_2400;
    return EZO_OK;
  case 9600:
    *baud_out = EZO_UART_POSIX_BAUD_9600;
    return EZO_OK;
  case 19200:
    *baud_out = EZO_UART_POSIX_BAUD_19200;
    return EZO_OK;
  case 38400:
    *baud_out = EZO_UART_POSIX_BAUD_38400;
    return EZO_OK;
  case 57600:
    *baud_out = EZO_UART_POSIX_BAUD_57600;
    return EZO_OK;
  case 115200:
    *baud_out = EZO_UART_POSIX_BAUD_115200;
    return EZO_OK;
  default:
    return EZO_ERR_INVALID_ARGUMENT;
  }
}

static void ezo_linux_i2c_device_reset(ezo_linux_i2c_device_t *device) {
  if (device == NULL) {
    return;
  }

  memset(device, 0, sizeof(*device));
  device->context.fd = -1;
}

static void ezo_linux_uart_device_reset(ezo_linux_uart_device_t *device) {
  if (device == NULL) {
    return;
  }

  memset(device, 0, sizeof(*device));
  device->serial.fd = -1;
}

ezo_result_t ezo_linux_i2c_device_open_path(ezo_linux_i2c_device_t *device,
                                            const char *path,
                                            uint8_t address) {
  int fd = -1;
  ezo_result_t result = EZO_OK;

  if (device == NULL || path == NULL || path[0] == '\0') {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  ezo_linux_i2c_device_reset(device);

  fd = open(path, O_RDWR);
  if (fd < 0) {
    return EZO_ERR_TRANSPORT;
  }

  result = ezo_linux_i2c_context_init(&device->context, fd);
  if (result != EZO_OK) {
    close(fd);
    return result;
  }

  result = ezo_device_init(&device->core, address, ezo_linux_i2c_transport(), &device->context);
  if (result != EZO_OK) {
    close(fd);
    ezo_linux_i2c_device_reset(device);
    return result;
  }

  device->owns_fd = 1;
  return EZO_OK;
}

ezo_result_t ezo_linux_i2c_device_open_bus(ezo_linux_i2c_device_t *device,
                                           uint32_t bus_index,
                                           uint8_t address) {
  char path[32];
  int written = 0;

  if (device == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  written = snprintf(path, sizeof(path), "/dev/i2c-%u", (unsigned int)bus_index);
  if (written <= 0 || (size_t)written >= sizeof(path)) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  return ezo_linux_i2c_device_open_path(device, path, address);
}

void ezo_linux_i2c_device_close(ezo_linux_i2c_device_t *device) {
  if (device == NULL) {
    return;
  }

  if (device->owns_fd && device->context.fd >= 0) {
    close(device->context.fd);
  }

  ezo_linux_i2c_device_reset(device);
}

ezo_i2c_device_t *ezo_linux_i2c_device_core(ezo_linux_i2c_device_t *device) {
  if (device == NULL) {
    return NULL;
  }

  return &device->core;
}

ezo_result_t ezo_linux_uart_device_open(ezo_linux_uart_device_t *device,
                                        const char *path,
                                        uint32_t baud_rate,
                                        uint32_t read_timeout_ms) {
  ezo_uart_posix_baud_t baud = EZO_UART_POSIX_BAUD_9600;
  ezo_result_t result = EZO_OK;

  if (device == NULL) {
    return EZO_ERR_INVALID_ARGUMENT;
  }

  ezo_linux_uart_device_reset(device);

  result = ezo_linux_uart_map_baud_rate(baud_rate, &baud);
  if (result != EZO_OK) {
    return result;
  }

  result = ezo_uart_posix_serial_open(&device->serial, path, baud, read_timeout_ms);
  if (result != EZO_OK) {
    ezo_linux_uart_device_reset(device);
    return result;
  }

  result = ezo_uart_device_init(&device->core, ezo_uart_posix_serial_transport(), &device->serial);
  if (result != EZO_OK) {
    ezo_uart_posix_serial_close(&device->serial);
    ezo_linux_uart_device_reset(device);
    return result;
  }

  return EZO_OK;
}

void ezo_linux_uart_device_close(ezo_linux_uart_device_t *device) {
  if (device == NULL) {
    return;
  }

  ezo_uart_posix_serial_close(&device->serial);
  ezo_linux_uart_device_reset(device);
}

ezo_uart_device_t *ezo_linux_uart_device_core(ezo_linux_uart_device_t *device) {
  if (device == NULL) {
    return NULL;
  }

  return &device->core;
}
