#include <log.h>
#include <main.h>
#include "experiments.h"
#include "stm32h7xx_hal.h"

bool uart_experiment(char *command, uint16_t len) {
    if (len < 2) {
        log_error("The command is too small to parse");
        return true;
    }

    if (command[0] == 'p') {
        // Power on/off
        if (command[1] == '1') {
            HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
            log_info("Output set ON");
        } else if (command[1] == '0') {
            HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);
            log_info("Output set OFF");
        } else {
            return false;
        }

        return true;
    }
}
