#ifndef RTB_SOFTWARE_PARAMETERS_H
#define RTB_SOFTWARE_PARAMETERS_H

#include <stdbool.h>
#include <stdint.h>

extern double floating_parameters[2];
extern uint32_t integer_parameters[2];

bool uart_set_parameter(char* command, uint16_t len);

#endif //RTB_SOFTWARE_PARAMETERS_H
