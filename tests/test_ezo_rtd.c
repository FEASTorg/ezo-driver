#include "ezo_rtd.h"
#include "tests/fakes/ezo_fake_transport.h"
#include "tests/fakes/ezo_fake_uart_transport.h"

#include <assert.h>
#include <string.h>

static void test_parse_helpers_cover_reading_and_queries(void) {
  ezo_rtd_reading_t reading;
  ezo_rtd_scale_status_t scale;
  ezo_rtd_calibration_status_t calibration;

  assert(ezo_rtd_parse_reading("25.104",
                               strlen("25.104"),
                               EZO_RTD_SCALE_CELSIUS,
                               &reading) == EZO_OK);
  assert(reading.temperature > 25.10 && reading.temperature < 25.11);
  assert(reading.scale == EZO_RTD_SCALE_CELSIUS);

  assert(ezo_rtd_parse_scale("?S,k", strlen("?S,k"), &scale) == EZO_OK);
  assert(scale.scale == EZO_RTD_SCALE_KELVIN);

  assert(ezo_rtd_parse_calibration_status("?Cal,1", strlen("?Cal,1"), &calibration) == EZO_OK);
  assert(calibration.calibrated == 1);
}

static void test_command_builders_format_expected_commands(void) {
  char command[32];

  assert(ezo_rtd_build_scale_command(command, sizeof(command), EZO_RTD_SCALE_FAHRENHEIT) ==
         EZO_OK);
  assert(strcmp(command, "S,f") == 0);

  assert(ezo_rtd_build_calibration_command(command, sizeof(command), 100.0, 2) == EZO_OK);
  assert(strcmp(command, "Cal,100.00") == 0);
}

static void test_i2c_helpers_send_and_parse_typed_responses(void) {
  static const uint8_t scale_response[] = {1, '?', 'S', ',', 'f', 0};
  ezo_fake_transport_t fake;
  ezo_i2c_device_t device;
  ezo_timing_hint_t hint;
  ezo_rtd_scale_status_t scale;
  ezo_rtd_reading_t reading;

  ezo_fake_transport_init(&fake);
  assert(ezo_device_init(&device, 102, ezo_fake_transport_vtable(), &fake) == EZO_OK);

  assert(ezo_rtd_send_read_i2c(&device, &hint) == EZO_OK);
  assert(hint.wait_ms == 600);
  assert(fake.last_tx_len == 1);
  assert(fake.last_tx_bytes[0] == 'r');

  ezo_fake_transport_set_response(&fake, (const uint8_t[]){1, '7', '7', '.', '0', 0}, 6);
  assert(ezo_rtd_read_response_i2c(&device, EZO_RTD_SCALE_FAHRENHEIT, &reading) == EZO_OK);
  assert(reading.temperature > 76.9 && reading.temperature < 77.1);
  assert(reading.scale == EZO_RTD_SCALE_FAHRENHEIT);

  ezo_fake_transport_set_response(&fake, scale_response, sizeof(scale_response));
  assert(ezo_rtd_send_scale_query_i2c(&device, &hint) == EZO_OK);
  assert(hint.wait_ms == 300);
  assert(ezo_rtd_read_scale_i2c(&device, &scale) == EZO_OK);
  assert(scale.scale == EZO_RTD_SCALE_FAHRENHEIT);
}

static void test_uart_helpers_cover_plain_read_and_query_sequences(void) {
  static const uint8_t read_response[] = {'2', '5', '.', '1', '0', '4', '\r'};
  static const uint8_t scale_response[] = {'?', 'S', ',', 'c', '\r', '*', 'O', 'K', '\r'};
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_timing_hint_t hint;
  ezo_rtd_reading_t reading;
  ezo_rtd_scale_status_t scale;

  ezo_fake_uart_transport_init(&fake);
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);

  assert(ezo_rtd_send_scale_set_uart(&device, EZO_RTD_SCALE_KELVIN, &hint) == EZO_OK);
  assert(hint.wait_ms == 300);
  assert(fake.tx_len == strlen("S,k\r"));
  assert(memcmp(fake.tx_bytes, "S,k\r", strlen("S,k\r")) == 0);

  ezo_fake_uart_transport_set_response(&fake, read_response, sizeof(read_response));
  assert(ezo_rtd_send_read_uart(&device, &hint) == EZO_OK);
  assert(hint.wait_ms == 1000);
  assert(ezo_rtd_read_response_uart(&device, EZO_RTD_SCALE_CELSIUS, &reading) == EZO_OK);
  assert(reading.temperature > 25.10 && reading.temperature < 25.11);
  assert(reading.scale == EZO_RTD_SCALE_CELSIUS);

  ezo_fake_uart_transport_set_response(&fake, scale_response, sizeof(scale_response));
  assert(ezo_rtd_send_scale_query_uart(&device, &hint) == EZO_OK);
  assert(hint.wait_ms == 300);
  assert(ezo_rtd_read_scale_uart(&device, &scale) == EZO_OK);
  assert(scale.scale == EZO_RTD_SCALE_CELSIUS);
}

int main(void) {
  test_parse_helpers_cover_reading_and_queries();
  test_command_builders_format_expected_commands();
  test_i2c_helpers_send_and_parse_typed_responses();
  test_uart_helpers_cover_plain_read_and_query_sequences();
  return 0;
}
