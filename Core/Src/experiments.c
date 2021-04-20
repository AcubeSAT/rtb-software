#include <log.h>
#include <main.h>
#include <stdlib.h>
#include <can.h>
#include <lcl.h>
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
            Outputs_ON();
        } else if (command[1] == '0') {
            Outputs_OFF();
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
                    break;
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
                        break;
                    default:
                        log_info("Stopped no associated experimental procedure");
                }

                currentExperiment = -1;
            }
        } else if (command[1] == 'r') {
            if (len < 3) {
                log_error("The command is too small to parse");
                return true;
            }

            // Reset experiment
            switch (strtol(command + 2, NULL, 10)) {
                case 1:
                    log_info("Resetting experiment %ld", currentExperiment);
                    Experiment_CAN_Reset();
                    break;
                default:
                    log_info("Reset no associated experimental procedure");
            }

            currentExperiment = -1;
        }  else {
            return false;
        }

        return true;
    }
}
