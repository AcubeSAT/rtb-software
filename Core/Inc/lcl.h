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
    HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_RESET);
    log_trace("LCL Setting");
    while (!HAL_GPIO_ReadPin(LCL_OUT_GPIO_Port, LCL_OUT_Pin)) {}
    HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_SET);
    log_trace("LCL SET");
    output_status = true;
}

inline void LCL_ON() {
    HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCL_SET_GPIO_Port, LCL_SET_Pin, GPIO_PIN_SET);
    log_trace("LCL SET");
    output_status = true;
}

inline void LCL_OFF() {
    HAL_GPIO_WritePin(LCL_RESET_GPIO_Port, LCL_RESET_Pin, GPIO_PIN_RESET);
    log_trace("LCL RESET");
    HAL_GPIO_WritePin(LCL_RESET_GPIO_Port, LCL_RESET_Pin, GPIO_PIN_SET);
    output_status = false;
}

inline void Outputs_ON() {
    Relay_ON();
    LCL_ON();
}

inline void Outputs_OFF() {
    LCL_OFF();
    Relay_OFF();
}

#endif //RTB_SOFTWARE_LCL_H
