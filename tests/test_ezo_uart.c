#include "ezo.h"
#include "ezo_uart.h"
#include "tests/fakes/ezo_fake_uart_transport.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

static void test_uart_send_command_appends_carriage_return(void) {
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_timing_hint_t hint;

  ezo_fake_uart_transport_init(&fake);
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_send_command(&device, "i", EZO_COMMAND_GENERIC, &hint) == EZO_OK);
  assert(hint.wait_ms == 300);
  assert(fake.write_call_count == 2);
  assert(fake.tx_len == 2);
  assert(memcmp(fake.tx_bytes, "i\r", 2) == 0);
}

static void test_uart_send_command_with_float_formats_value(void) {
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_timing_hint_t hint;

  ezo_fake_uart_transport_init(&fake);
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_send_command_with_float(&device,
                                          "rt,",
                                          25.0,
                                          3,
                                          EZO_COMMAND_READ_WITH_TEMP_COMP,
                                          &hint) == EZO_OK);
  assert(hint.wait_ms == 1000);
  assert(fake.tx_len == strlen("rt,25.000\r"));
  assert(memcmp(fake.tx_bytes, "rt,25.000\r", strlen("rt,25.000\r")) == 0);
}

static void test_uart_send_command_rejects_embedded_terminator(void) {
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;

  ezo_fake_uart_transport_init(&fake);
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_send_command(&device, "r\n", EZO_COMMAND_READ, NULL) ==
         EZO_ERR_INVALID_ARGUMENT);
}

static void test_uart_read_response_classifies_data_and_parses(void) {
  static const uint8_t response[] = {'1', '2', '.', '3', '4', '\r'};
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_uart_response_kind_t kind = EZO_UART_RESPONSE_UNKNOWN;
  char buffer[16];
  size_t response_len = 0;
  double value = 0.0;

  ezo_fake_uart_transport_init(&fake);
  fake.max_bytes_per_read = 2;
  ezo_fake_uart_transport_set_response(&fake, response, sizeof(response));
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);

  assert(ezo_uart_read_response(&device, buffer, sizeof(buffer), &response_len, &kind) ==
         EZO_OK);
  assert(kind == EZO_UART_RESPONSE_DATA);
  assert(ezo_uart_device_get_last_response_kind(&device) == EZO_UART_RESPONSE_DATA);
  assert(response_len == 5);
  assert(memcmp(buffer, "12.34", response_len) == 0);
  assert(ezo_parse_double(buffer, response_len, &value) == EZO_OK);
  assert(value > 12.33 && value < 12.35);
}

static void test_uart_read_response_classifies_ok(void) {
  static const uint8_t response[] = {'*', 'O', 'K', '\r'};
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_uart_response_kind_t kind = EZO_UART_RESPONSE_UNKNOWN;
  char buffer[8];
  size_t response_len = 0;

  ezo_fake_uart_transport_init(&fake);
  ezo_fake_uart_transport_set_response(&fake, response, sizeof(response));
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_read_response(&device, buffer, sizeof(buffer), &response_len, &kind) ==
         EZO_OK);
  assert(kind == EZO_UART_RESPONSE_OK);
  assert(response_len == 3);
  assert(memcmp(buffer, "*OK", 3) == 0);
}

static void test_uart_read_response_classifies_error(void) {
  static const uint8_t response[] = {'*', 'E', 'R', '\r'};
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_uart_response_kind_t kind = EZO_UART_RESPONSE_UNKNOWN;
  char buffer[8];
  size_t response_len = 0;

  ezo_fake_uart_transport_init(&fake);
  ezo_fake_uart_transport_set_response(&fake, response, sizeof(response));
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_read_response(&device, buffer, sizeof(buffer), &response_len, &kind) ==
         EZO_OK);
  assert(kind == EZO_UART_RESPONSE_ERROR);
  assert(response_len == 3);
  assert(memcmp(buffer, "*ER", 3) == 0);
}

static void test_uart_read_response_rejects_incomplete_line(void) {
  static const uint8_t response[] = {'1', '2', '.', '3', '4'};
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_uart_response_kind_t kind = EZO_UART_RESPONSE_UNKNOWN;
  char buffer[16];
  size_t response_len = 0;

  ezo_fake_uart_transport_init(&fake);
  ezo_fake_uart_transport_set_response(&fake, response, sizeof(response));
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_read_response(&device, buffer, sizeof(buffer), &response_len, &kind) ==
         EZO_ERR_PROTOCOL);
  assert(kind == EZO_UART_RESPONSE_UNKNOWN);
  assert(response_len == 0);
}

static void test_uart_read_response_detects_buffer_too_small(void) {
  static const uint8_t response[] = {'1', '2', '3', '4', '\r'};
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_uart_response_kind_t kind = EZO_UART_RESPONSE_UNKNOWN;
  char buffer[4];
  size_t response_len = 0;

  ezo_fake_uart_transport_init(&fake);
  ezo_fake_uart_transport_set_response(&fake, response, sizeof(response));
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_read_response(&device, buffer, sizeof(buffer), &response_len, &kind) ==
         EZO_ERR_BUFFER_TOO_SMALL);
  assert(kind == EZO_UART_RESPONSE_UNKNOWN);
  assert(response_len == 0);
}

static void test_uart_send_command_propagates_transport_failure(void) {
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;

  ezo_fake_uart_transport_init(&fake);
  fake.write_result = EZO_ERR_TRANSPORT;
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_send_command(&device, "status", EZO_COMMAND_GENERIC, NULL) ==
         EZO_ERR_TRANSPORT);
}

static void test_uart_read_response_propagates_transport_failure(void) {
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;
  ezo_uart_response_kind_t kind = EZO_UART_RESPONSE_UNKNOWN;
  char buffer[8];
  size_t response_len = 0;

  ezo_fake_uart_transport_init(&fake);
  fake.read_result = EZO_ERR_TRANSPORT;
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_read_response(&device, buffer, sizeof(buffer), &response_len, &kind) ==
         EZO_ERR_TRANSPORT);
  assert(kind == EZO_UART_RESPONSE_UNKNOWN);
  assert(response_len == 0);
}

static void test_uart_discard_input_uses_optional_transport_hook(void) {
  static const uint8_t response[] = {'1', '2', '\r'};
  ezo_fake_uart_transport_t fake;
  ezo_uart_device_t device;

  ezo_fake_uart_transport_init(&fake);
  ezo_fake_uart_transport_set_response(&fake, response, sizeof(response));
  assert(ezo_uart_device_init(&device, ezo_fake_uart_transport_vtable(), &fake) == EZO_OK);
  assert(ezo_uart_discard_input(&device) == EZO_OK);
  assert(fake.discard_call_count == 1);
  assert(fake.response_offset == fake.response_len);
}

int main(void) {
  test_uart_send_command_appends_carriage_return();
  test_uart_send_command_with_float_formats_value();
  test_uart_send_command_rejects_embedded_terminator();
  test_uart_read_response_classifies_data_and_parses();
  test_uart_read_response_classifies_ok();
  test_uart_read_response_classifies_error();
  test_uart_read_response_rejects_incomplete_line();
  test_uart_read_response_detects_buffer_too_small();
  test_uart_send_command_propagates_transport_failure();
  test_uart_read_response_propagates_transport_failure();
  test_uart_discard_input_uses_optional_transport_hook();
  return 0;
}
