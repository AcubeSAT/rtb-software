#include <log.h>
#include <main.h>
#include <stdlib.h>
#include <can.h>
#include <lcl.h>
#include <mram.h>
#include <shift_register.h>
#include <characterisation.h>
#include "experiments.h"
#include "stm32h7xx_hal.h"

int currentExperiment = -1;

void start_experiment(int experiment) {
    if (experiment < 0) {
        experiment = currentExperiment;
    }

    log_debug("Starting experiment %ld", experiment);

    switch (experiment) {
        case 1:
        case 2:
            Experiment_CAN_Start();
            break;
        case 3:
            Experiment_MRAM_Start();
            break;
        case 4:
            Experiment_REG_Start();
            break;
        case 10:
            Experiment_Characterisation_Start();
            break;
        default:
            log_trace("No associated experimental procedure");
    }
}

void stop_experiment(int experiment) {
    if (experiment < 0) {
        experiment = currentExperiment;
    }

    log_debug("Pausing experiment %ld", experiment);

    switch (experiment) {
        case 1:
        case 2:
            Experiment_CAN_Stop();
            break;
        case 3:
            Experiment_MRAM_Stop();
            break;
        case 4:
            Experiment_REG_Stop();
            break;
        case 10:
            Experiment_Characterisation_Stop();
            break;
        default:
            log_trace("No associated experimental procedure");
    }
}

void reset_experiment(int experiment) {
    if (experiment < 0) {
        experiment = currentExperiment;
    }

    log_info("Resetting experiment %ld", experiment);

    switch (experiment) {
        case 1:
        case 2:
            Experiment_CAN_Reset();
            break;
        case 3:
            Experiment_MRAM_Reset();
            break;
        case 4:
            Experiment_REG_Reset();
            break;
        case 10:
            Experiment_Characterisation_Reset();
            break;
        default:
            log_trace("No associated experimental procedure");
    }
}

void loop_experiment(int experiment) {
    if (experiment < 0) {
        experiment = currentExperiment;
    }

    switch (experiment) {
        case 1:
            Experiment_CAN_Loop();
            break;
        case 2:
            Experiment_CAN_Silent_Loop();
            break;
        case 3:
            Experiment_MRAM_Loop();
            break;
        case 4:
            Experiment_REG_Loop();
            break;
        case 10:
            Experiment_Characterisation_Loop();
            break;
    }
}

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
        } else if (command[1] == '2') {
            LCL_Power_Cycle();
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
            state = none;
            currentExperiment = strtol(command + 2, NULL, 10);
            start_experiment(currentExperiment);
        } else if (command[1] == 'p') {
            // Stop experiment
            if (currentExperiment < 0) {
                log_warn("Experiment is already stopped");
                return true;
            } else {
                int previousExperiment = currentExperiment;
                currentExperiment = -1;

                stop_experiment(previousExperiment);
            }
        } else if (command[1] == 'r') {
            if (len < 3) {
                log_error("The command is too small to parse");
                return true;
            }

            int resetExperiment = strtol(command + 2, NULL, 10);
            reset_experiment(resetExperiment);
            currentExperiment = -1;
        }  else {
            return false;
        }

        return true;
    }
}
