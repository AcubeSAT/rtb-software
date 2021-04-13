#include <log.h>
#include <main.h>
#include <stdlib.h>
#include <can.h>
#include "experiments.h"
#include "stm32h7xx_hal.h"

int currentExperiment = -1;

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
    } else if (command[0] == 'e') {
        if (command[1] == 't') {
            if (len < 3) {
                log_error("The command is too small to parse");
                return true;
            }

            // Start experiment

            currentExperiment = strtol(command + 2, NULL, 10);
            switch (currentExperiment) {
                case 1:
                    log_info("Starting experiment %ld", currentExperiment);
                    Experiment_CAN_Start();
                default:
                    log_info("No associated experimental procedure");
            }
        } else if (command[1] == 'p') {
            // Stop experiment
            if (currentExperiment < 0) {
                log_warn("Experiment is already stopped");
                return true;
            } else {
                switch (currentExperiment) {
                    case 1:
                        log_info("Stopping experiment %ld", currentExperiment);
                        Experiment_CAN_Stop();
                    default:
                        log_info("Stopped no associated experimental procedure");
                }

                currentExperiment = -1;
            }
        } else {
            return false;
        }

        return true;
    }
}
