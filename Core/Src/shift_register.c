#include "shift_register.h"

#include <stdint.h>
#include <stm32h7xx_hal.h>
#include <log.h>
#include <stdlib.h>
#include "main.h"
#include "mram.h"

static bool experimentStarted = false;

void Experiment_REG_Start() {
    experimentStarted = true;
}

void Experiment_REG_Stop() {
    experimentStarted = false;

    HAL_GPIO_WritePin(SR_RCLK_GPIO_Port, SR_RCLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SR_SRCLK_GPIO_Port, SR_SRCLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SR_SER_GPIO_Port, SR_SER_Pin, GPIO_PIN_RESET);
}

void Experiment_REG_Loop() {
    log_debug("REG loop: start");

    // ==== Serial output
    // WRITE 0 TO ALL
    HAL_GPIO_WritePin(SR_SER_GPIO_Port, SR_SER_Pin, GPIO_PIN_SET);

    // Write all pins to the selected value
    for (int i = 0; i < 8; i++) {
        HAL_Delay(1);
        HAL_GPIO_WritePin(SR_SRCLK_GPIO_Port, SR_SRCLK_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(SR_SRCLK_GPIO_Port, SR_SRCLK_Pin, GPIO_PIN_RESET);
    }

    // Output clock pulse
    HAL_Delay(1);
    HAL_GPIO_WritePin(SR_RCLK_GPIO_Port, SR_RCLK_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(SR_RCLK_GPIO_Port, SR_RCLK_Pin, GPIO_PIN_RESET);

    log_debug("REG loop: done");

    while (experimentStarted) {
        // Don't set pins again, we just set them once.
        // One can replace this while loop with a delay to have pins be reset again and again
    }

    Experiment_REG_Stop(); // For good measure
}

void Experiment_REG_Reset() {
    // Not a lot of interesting stuff here
    Experiment_REG_Stop();
}
