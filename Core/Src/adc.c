#include <stdint.h>
#include <stm32h7xx_hal.h>
#include <parameters.h>
#include <stdio.h>
#include <main.h>
#include "adc.h"

// Add some extra values for leeway
uint32_t last_converted_values[100];
uint32_t converted_values[100];

uint8_t current_conversion = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    last_converted_values[current_conversion] = converted_values[current_conversion];
    converted_values[current_conversion++] = HAL_ADC_GetValue(hadc);

    if (current_conversion == 3) {
        // All ADC conversions completed
        if (enum_parameters[TakeMeasurements]) {
            printf(
                    UART_CONTROL UART_C_MEASUREMENT "%f %f %f %d\r\n",
                    converted_values[0] * floating_parameters[0] / 65535,
                    converted_values[1] * floating_parameters[0] / 65535,
                    converted_values[2] * floating_parameters[0] / 65535,
                    HAL_GPIO_ReadPin(LCL_OUT_GPIO_Port, LCL_OUT_Pin)
            );
        }
    }
}

void ADC_BeginConversion() {
    current_conversion = 0;
    HAL_ADC_Start_IT(&hadc1);
}