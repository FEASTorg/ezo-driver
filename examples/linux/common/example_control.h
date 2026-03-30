#ifndef EZO_EXAMPLE_CONTROL_H
#define EZO_EXAMPLE_CONTROL_H

#include "ezo_control.h"

#ifdef __cplusplus
extern "C" {
#endif

ezo_result_t ezo_example_query_info_i2c(ezo_i2c_device_t *device, ezo_device_info_t *info_out);

ezo_result_t ezo_example_print_shared_control_i2c(ezo_i2c_device_t *device,
                                                  ezo_product_id_t product_id);

#ifdef __cplusplus
}
#endif

#endif
