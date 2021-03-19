#include "parameters.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <log.h>
#include <stdlib.h>

double floating_parameters[] = {
        0.0f,
        0.0f,
};
uint32_t integer_parameters[] = {
        0,
        0
};

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
        } else {
            // Floating parameter
            double new_value = strtod(command_value + 1, NULL);

            if (parameter_number >= sizeof(floating_parameters) / sizeof(*floating_parameters)) {
                log_error("This parameter does not exist");
                return true;
            }

            floating_parameters[parameter_number] = new_value;
            log_debug("Set floating parameter %ld to %lf", parameter_number, new_value);
        }
    } else {
        return false;
    }

    return true;
}
