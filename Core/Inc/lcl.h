#ifndef RTB_SOFTWARE_LCL_H
#define RTB_SOFTWARE_LCL_H

#include <log.h>
#include "main.h"

extern bool output_status;

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

    log_trace("LCL SET");
    output_status = true;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

inline void LCL_ON() {
    // Initialise timer 14, which when completed, will push an interrupt to LCL_ON_FORCE() to actually enable the LCLs
    __HAL_TIM_DISABLE(&htim14);
    __HAL_TIM_SET_COUNTER(&htim14, 0); // Make sure to start from the start
    __HAL_TIM_ENABLE(&htim14);
}

inline void LCL_OFF() {
    TIM_CCxChannelCmd(htim8.Instance, TIM_CHANNEL_3, TIM_CCx_DISABLE);
    TIM_CCxChannelCmd(htim8.Instance, TIM_CHANNEL_4, TIM_CCx_ENABLE);
    __HAL_TIM_ENABLE(&htim8);

    log_trace("LCL RESET");

    output_status = false;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}

inline void Outputs_ON() {
    Relay_ON();
    LCL_ON();
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

#endif //RTB_SOFTWARE_LCL_H
