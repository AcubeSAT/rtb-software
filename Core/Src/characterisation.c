#include <stdint.h>
#include <log.h>
#include <parameters.h>
#include <lcl.h>
#include "characterisation.h"
#include "main.h"
#include "adc.h"

GPIO_TypeDef* measurementPortsGPIO[8] = {
        MEAS_0_GPIO_Port,
        MEAS_1_GPIO_Port,
        MEAS_2_GPIO_Port,
        MEAS_3_GPIO_Port,
        MEAS_4_GPIO_Port,
        MEAS_5_GPIO_Port,
        MEAS_6_GPIO_Port,
        MEAS_7_GPIO_Port,
};

uint32_t measurementPins[8] = {
        MEAS_0_Pin,
        MEAS_1_Pin,
        MEAS_2_Pin,
        MEAS_3_Pin,
        MEAS_4_Pin,
        MEAS_5_Pin,
        MEAS_6_Pin,
        MEAS_7_Pin,
};

const float measurementResistors[8] = {
        1600,
        910,
        430,
        200,
        120,
        51,
        27,
        15
};

const float constVoltage = 3.4f;

uint8_t sequence = 0;

bool start = false;

float get_current() {
    float voltage = converted_values[2] * floating_parameters[0] / 65535;
//    log_trace("%f", voltage);
    float current = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t bit = 1 == ( (sequence >> i) & 1);
        if (bit) {
            current += voltage / measurementResistors[i];
        }
    }
    return current * 1000;
}

void Experiment_Characterisation_Next() {
    floating_parameters[1] += 0.05;
    if (floating_parameters[1] > 3.3) {
        log_info("Experiment done!!");
        stop_experiment(-1);
    } else {
        callback_dac_output(1, &(floating_parameters[1]));
        log_debug("Set threshold to %f V", floating_parameters[1]);
    }
}

void Experiment_Characterisation_Start() {
    start = true;
}

void Experiment_Characterisation_Loop() {
    if (!output_status || !start) return;

    for (int i = 0; i < 8; i++) {
        uint8_t bit = 1 == ( (sequence >> i) & 1);
        HAL_GPIO_WritePin(measurementPortsGPIO[i], measurementPins[i], bit);
    }

    HAL_Delay(1000/20*2);
//    HAL_Delay(20);

    log_trace("Current is %f mA", get_current());

    sequence++;

    if (sequence == 0) {
        Experiment_Characterisation_Next();
    }
}

void Experiment_Characterisation_Stop() {
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(measurementPortsGPIO[i], measurementPins[i], GPIO_PIN_RESET);
    }

    float threshold = floating_parameters[2];
    float sense = last_converted_values[0] * floating_parameters[0] / 65535;
    log_info("SEL at %f %f V - %f mA", threshold, sense, get_current());

    start = false;

    Experiment_Characterisation_Next();

    sequence = 0;
}

void Experiment_Characterisation_Reset() {
    sequence = 0;
}


