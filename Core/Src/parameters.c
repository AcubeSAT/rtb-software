#include "parameters.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <log.h>
#include <stdlib.h>
#include <stm32h7xx_hal.h>
#include "main.h"

enum floating_parameter_names {
    MaxVoltage = 0,
    Dac1 = 1,
    Dac2,
};

double floating_parameters[] = {
        3.3f,
        0.0f,
        0.2f,
};
uint32_t integer_parameters[] = {
        0,
        0
};

enum integer_parameter_names {
    Test1 = 0,
    Test2 = 1,
};

floating_callback floating_callbacks[] = {
    NULL,
    callback_dac_output,
    callback_dac_output
};
integer_callback integer_callbacks[] = {
    NULL,
    NULL
};

void callback_dac_output(uint32_t parameter, double * value) {
    uint8_t parsed_value = *value / floating_parameters[MaxVoltage] * 255;

    log_trace("Setting DAC %ld to %d", parameter, parsed_value);

    HAL_DAC_SetValue(&hdac1, parameter == 1 ? DAC_CHANNEL_1 : DAC_CHANNEL_2, DAC_ALIGN_8B_R, parsed_value);
}

/**
 * Receive a UART string that sets a parameter to a value
 * @return True if this command was indeed a parameter command, false if it needs to be passed to another handler
 */
bool uart_set_parameter(char* command, uint16_t len) {
    if (len < 2) {
        log_error("The command is too small to parse");
        return true;
    } else if (len > 254) {
        log_error("The command is too large to parse");
        return true;
    }

    command[len] = '\0'; // A bit dangerous but that's ok

    if (command[0] == 'd' || command[0] == 'f') {
        char * command_value;
        uint32_t parameter_number = strtol(command + 1, &command_value, 10);

        if (command_value - command >= len) {
            log_error("Invalid command format");
            return true;
        }

        if (command[0] == 'd') {
            // Integer parameter
            uint32_t new_value = strtol(command_value + 1, NULL, 10);

            if (parameter_number >= sizeof(integer_parameters) / sizeof(*integer_parameters)) {
                log_error("This parameter does not exist");
                return true;
            }

            integer_parameters[parameter_number] = new_value;
            log_debug("Set integer parameter %ld to %ld", parameter_number, new_value);
            
            if (integer_callbacks[parameter_number] != NULL) {
                integer_callbacks[parameter_number](parameter_number, &(integer_parameters[parameter_number]));
            }
        } else {
            // Floating parameter
            double new_value = strtod(command_value + 1, NULL);

            if (parameter_number >= sizeof(floating_parameters) / sizeof(*floating_parameters)) {
                log_error("This parameter does not exist");
                return true;
            }

            floating_parameters[parameter_number] = new_value;
            log_debug("Set floating parameter %ld to %lf", parameter_number, new_value);

            if (floating_callbacks[parameter_number] != NULL) {
                floating_callbacks[parameter_number](parameter_number, &(floating_parameters[parameter_number]));
            }
        }
    } else {
        return false;
    }

    return true;
}
