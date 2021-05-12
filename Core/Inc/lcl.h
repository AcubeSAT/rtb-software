#ifndef RTB_SOFTWARE_LCL_H
#define RTB_SOFTWARE_LCL_H

#include <log.h>
#include "main.h"
#include "experiments.h"

extern bool output_cycling;
extern bool output_status;

extern bool __alsoStartExperiment;

inline void Relay_ON() {
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
}

inline void Relay_OFF() {
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);
}

inline void LCL_ON_Force() {
    __HAL_TIM_DISABLE(&htim8);
    TIM_CCxChannelCmd(htim8.Instance, TIM_CHANNEL_3, TIM_CCx_ENABLE);
    TIM_CCxChannelCmd(htim8.Instance, TIM_CHANNEL_4, TIM_CCx_DISABLE);
    __HAL_TIM_ENABLE(&htim8);

    printf(UART_CONTROL UART_C_POWER "1" "\r\n"); // TODO: Actually use ON input to determine this
    log_trace("LCL SET");
    output_status = true;
    output_cycling = false;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

    if (__alsoStartExperiment) {
        __alsoStartExperiment = false;
        start_experiment(-1);
    }
}

inline void LCL_ON() {
    output_cycling = true;

    // Initialise timer 14, which when completed, will push an interrupt to LCL_ON_FORCE() to actually enable the LCLs
    __HAL_TIM_DISABLE(&htim14);
    __HAL_TIM_SET_COUNTER(&htim14, 0); // Make sure to start from the start
    __HAL_TIM_ENABLE(&htim14);
}

inline void LCL_ON_Experiment() {
    __alsoStartExperiment = true;
    LCL_ON();
}

inline void LCL_OFF() {
    output_status = false;

    TIM_CCxChannelCmd(htim8.Instance, TIM_CHANNEL_3, TIM_CCx_DISABLE);
    TIM_CCxChannelCmd(htim8.Instance, TIM_CHANNEL_4, TIM_CCx_ENABLE);
    __HAL_TIM_ENABLE(&htim8);

    log_trace("LCL RESET");

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}

inline void LCL_Power_Cycle() {
    LCL_OFF();
    LCL_ON();

    printf(UART_CONTROL UART_C_POWER "0" "PC" "\r\n");
}

inline void Outputs_ON() {
    Relay_ON();
    LCL_ON_Force();
}

inline void Outputs_OFF() {
    LCL_OFF();
    Relay_OFF();
}

inline void LCL_Test_Trigger() {
    // The timer is busy whenever the base starts and I don't want to bother with interrupts, so we do this the hard way
    __HAL_TIM_ENABLE(&htim3);
    TIM_CCxChannelCmd(htim3.Instance, TIM_CHANNEL_2, TIM_CCx_ENABLE);
}

inline void LCL_Test_Trigger_OFF() {
    __HAL_TIM_SET_COUNTER(&htim3, 60000);
    __HAL_TIM_DISABLE(&htim3);
    TIM_CCxChannelCmd(htim3.Instance, TIM_CHANNEL_2, TIM_CCx_DISABLE);
    // Reset counter so that next time starts from 0
    __HAL_TIM_SET_COUNTER(&htim3, 0);
}

inline void LCL_Check() {
    if (output_status) { // Expecting output to be true
        bool actual_output = HAL_GPIO_ReadPin(LCL_OUT_GPIO_Port, LCL_OUT_Pin);

        if (!actual_output) {
            log_error("LCL failure to set. Retrying");
            LCL_ON();
        }
    }
}

#endif //RTB_SOFTWARE_LCL_H
