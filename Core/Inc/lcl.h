#ifndef RTB_SOFTWARE_LCL_H
#define RTB_SOFTWARE_LCL_H

#include "main.h"

extern bool output_status;

inline void Relay_ON() {
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
    log_info("Output relay set ON");
}

inline void Relay_OFF() {
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);
    log_info("Output relay set OFF");
}

inline void LCL_ON_Force() {
//    HAL_Delay(1);

    HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_RESET);
    log_trace("LCL Setting");

    uint32_t timeout_counter = HAL_GetTick();
    while (!HAL_GPIO_ReadPin(LCL_OUT_GPIO_Port, LCL_OUT_Pin)) {
        if (HAL_GetTick() > timeout_counter + 1500) {
            log_error("LCL SET timeout");
            HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_SET);
            return;
        }
    }

    HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    log_trace("LCL SET");
    output_status = true;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

inline void LCL_ON() {
    HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_SET);

    log_trace("LCL SET");
    output_status = true;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

inline void LCL_OFF() {
    HAL_GPIO_WritePin(LCL_RESET_GPIO_Port, LCL_RESET_Pin, GPIO_PIN_RESET);
    log_trace("LCL RESET");
    HAL_GPIO_WritePin(LCL_RESET_GPIO_Port, LCL_RESET_Pin, GPIO_PIN_SET);

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
