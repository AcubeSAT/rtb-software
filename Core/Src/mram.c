#include <stdint.h>
#include <stm32h7xx_hal.h>
#include <log.h>
#include <stdlib.h>
#include "main.h"
#include "mram.h"
#include "parameters.h"

static bool experimentStarted = false;
static uint32_t fillAddress = 0;
static uint32_t verifyAddress = 0;

#define ofast __attribute__((optimize("-Ofast")))

const uint32_t mask = (1 << 15) | (1 << 0) | (1 << 1) | (1 << 2);
//const uint32_t mask = (1 << 15);
const uint32_t MRAM_max_address = 2097151;

static struct Stats {
    uint64_t bytesWritten;
    uint64_t loops;
} stats = { 0, 0};

// Source: https://github.com/csknk/fast-modular-exponentiation
 ofast static inline uint64_t powm_u64(uint64_t b, uint64_t e, uint64_t m)
{
    uint64_t result = 1;

    if (1 & e) {
        result = b;
    }

    while (1) {
        if (!e) break;
        e >>= 1;
        b = (b * b) % m;
        if (e & 1) {
            result = (result * b) % m;
        }
    }
    return result;
}

ofast static inline uint32_t powm_u32(uint32_t b, uint32_t e, uint32_t m)
{
    uint32_t result = 1;

    if (1 & e) {
        result = b;
    }

    while (1) {
        if (!e) break;
        e >>= 1;
        b = (b * b) % m;
        if (e & 1) {
            result = (result * b) % m;
        }
    }
    return result;
}

ofast static inline uint8_t powm_u8(uint8_t b, uint8_t e, uint8_t m)
{
    uint8_t result = 1;

    if (1 & e) {
        result = b;
    }

    while (1) {
        if (!e) break;
        e >>= 1;
        b = (b * b) % m;
        if (e & 1) {
            result = (result * b) % m;
        }
    }
    return result;
}

ofast static inline uint8_t MRAM_value(uint32_t address) {
    return (uint8_t) address;
}

static inline volatile uint8_t MRAM_read(uint32_t address) {
    return *((volatile uint8_t *) (0x60000000 + address));
}

static inline void MRAM_write(uint32_t address, uint8_t data) {
    *((volatile uint8_t *) (0x60000000 + address)) = data;
}

ofast inline static void MRAM_progress_report(char type, uint32_t fill, uint32_t max) {
    static int last_update = 0;
    static const int update_rate = 1000 / 30;

    if (HAL_GetTick() - last_update > update_rate || fill == 0 || fill == max) {
        last_update = HAL_GetTick();
        printf(UART_CONTROL UART_C_MEMPROGRESS "%c %ld %ld\r\n", type, fill, max);
    }
}

ofast static void MRAM_error_report(uint32_t address, uint8_t expected, uint8_t read1, const char* verification_stage) {
    printf(UART_CONTROL UART_C_MEMERROR "%x %x %x %s\r\n", (int) address, (int) expected, (int) read1, verification_stage);
 }

static void Experiment_MRAM_Statistics(bool force) {
    static uint32_t lastStats = 0;

    if (HAL_GetTick() - lastStats > 200 || force) {
        lastStats = HAL_GetTick();
        printf(UART_CONTROL UART_C_STATISTICS "MRAM %lld %lld\r\n", stats.bytesWritten, stats.loops);
    }
}

static void Experiment_MRAM_Fill() {
    log_debug("MRAM data fill");

    for (; fillAddress <= MRAM_max_address; fillAddress += 8) {
        if (!experimentStarted) break;

        uint64_t value = 0;
        for (uint64_t j = 0; j < 8; j++) {
            uint8_t value_8bit = MRAM_value((fillAddress + j) & ~mask);

            if (RANDOM_ERRORS && rand() % 99 == 0) {
                value_8bit |= rand();
            }

            value |= ((uint64_t) (value_8bit)) << (8 * j);
        }

        *((volatile uint64_t *) (0x60000000 + fillAddress)) = value;

        MRAM_progress_report('f', fillAddress, MRAM_max_address);
    }

    MRAM_progress_report('f', fillAddress, MRAM_max_address);
}

void Experiment_MRAM_Start() {
    HAL_SRAM_MspInit(&hsram1);
    experimentStarted = true;
}

void Experiment_MRAM_Stop() {
     // Do not deinitialise FMC since it causes crashes in case of memory access.
     // Instead, we just manually deinitialise the relevant pins
//    HAL_SRAM_MspDeInit(&hsram1);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_8
                           |GPIO_PIN_9|GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                           |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13
                           |GPIO_PIN_14|GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                           |GPIO_PIN_4|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                           |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4
                           |GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7);

    experimentStarted = false;
}

void Experiment_MRAM_Loop() {
    MRAM_progress_report('r', 0, MRAM_max_address);

    Experiment_MRAM_Fill();
    log_debug("MRAM verify loop: start");

    uint32_t errors = 0;

    uint32_t start_time = HAL_GetTick();
    uint32_t stop_time = HAL_GetTick();
    for (; verifyAddress <= MRAM_max_address; verifyAddress += 1) {
        if ((verifyAddress & mask) != 0) continue;
        if (!experimentStarted) break;

        stats.bytesWritten++;

        uint8_t expected_value = MRAM_value(verifyAddress);
        uint8_t read = MRAM_read(verifyAddress);

        // STEP 1: Read old value
        if (read != expected_value) {
            errors += 1;

            MRAM_error_report(verifyAddress, expected_value, read, "Read");
        }

        // STEP 2: Write + read new value
        uint8_t new_value = ~expected_value;
        MRAM_write(verifyAddress, new_value);
        uint8_t new_read = MRAM_read(verifyAddress);
        if (new_read != new_value || (RANDOM_ERRORS && rand() % 999 == 0)) {
            errors += 1;

            MRAM_error_report(verifyAddress, new_value, read, "Write");
        }

        MRAM_progress_report('r', verifyAddress, MRAM_max_address);
        Experiment_MRAM_Statistics(false);
    }

    MRAM_progress_report('r', verifyAddress, MRAM_max_address);

    stop_time = HAL_GetTick();
    stats.loops++;
    Experiment_MRAM_Statistics(true);

    log_info("MRAM verify loop: stop %ld/%ld errors [%ld]", errors, (2 * (MRAM_max_address + 1)) / (1 << __builtin_popcount(mask)), stop_time - start_time);

    if (verifyAddress >= MRAM_max_address) {
        fillAddress = 0;
        verifyAddress = 0;
    }
}

void Experiment_MRAM_Reset() {
    stats.bytesWritten = 0;
    stats.loops = 0;

    fillAddress = 0;
    verifyAddress = 0;
}
