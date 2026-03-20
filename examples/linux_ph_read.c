#include "ezo_i2c_linux_i2c.h"
#include "ezo_ph.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  const char *device_path = "/dev/i2c-1";
  int fd = -1;
  ezo_linux_i2c_context_t transport_context;
  ezo_i2c_device_t device;
  ezo_timing_hint_t hint;
  ezo_ph_reading_t reading;
  ezo_result_t result = EZO_OK;

  if (argc > 1) {
    device_path = argv[1];
  }

  fd = open(device_path, O_RDWR);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  result = ezo_linux_i2c_context_init(&transport_context, fd);
  if (result != EZO_OK) {
    close(fd);
    return 1;
  }

  result = ezo_device_init(&device, 99, ezo_linux_i2c_transport(), &transport_context);
  if (result != EZO_OK) {
    close(fd);
    return 1;
  }

  result = ezo_ph_send_read_i2c(&device, &hint);
  if (result != EZO_OK) {
    close(fd);
    return 1;
  }

  usleep((useconds_t)(hint.wait_ms * 1000U));

  result = ezo_ph_read_response_i2c(&device, &reading);
  close(fd);

  if (result != EZO_OK) {
    fprintf(stderr, "typed pH read failed: %d\n", (int)result);
    return 1;
  }

  printf("pH=%.3f\n", reading.ph);
  return 0;
}
