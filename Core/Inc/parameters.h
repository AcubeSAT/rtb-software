#ifndef RTB_SOFTWARE_PARAMETERS_H
#define RTB_SOFTWARE_PARAMETERS_H

#include <stdbool.h>
#include <stdint.h>

enum floating_parameter_names {
    MaxVoltage = 0,
    Dac1 = 1,
    Dac2,
};

enum integer_parameter_names {
    Test1 = 0,
    Test2 = 1,
};

enum enum_parameter_names {
    CANrate = 0,
    RandomErrors = 1,
    TakeMeasurements = 2,
};

extern double floating_parameters[3];
extern uint32_t integer_parameters[1];
extern uint32_t enum_parameters[3];

typedef void (*floating_callback)(uint32_t parameter, double * value);
typedef void (*integer_callback)(uint32_t parameter, uint32_t * value);

void callback_dac_output(uint32_t parameter, double * value);

bool uart_set_parameter(char* command, uint16_t len);

#define RANDOM_ERRORS (enum_parameters[RandomErrors] == 1)

#endif //RTB_SOFTWARE_PARAMETERS_H
