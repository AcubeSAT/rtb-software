#ifndef RTB_SOFTWARE_PARAMETERS_H
#define RTB_SOFTWARE_PARAMETERS_H

#include <stdbool.h>
#include <stdint.h>

extern double floating_parameters[2];
extern uint32_t integer_parameters[2];

typedef void (*floating_callback)(uint32_t parameter, double * value);
typedef void (*integer_callback)(uint32_t parameter, uint32_t * value);

void callback_dac_output(uint32_t parameter, double * value);

bool uart_set_parameter(char* command, uint16_t len);

#endif //RTB_SOFTWARE_PARAMETERS_H
