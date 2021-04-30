#include <stdint.h>
#include <stm32h7xx_hal.h>
#include <log.h>
#include "main.h"
#include "mram.h"

#define ofast __attribute__((optimize("-Ofast")))

const uint32_t mask = (1 << 15) | (1 << 0) | (1 << 1) | (1 << 2);
//const uint32_t mask = (1 << 15);
const uint32_t MRAM_max_address = 2097151;

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
    return ((((uint8_t) (address * 1163) + 89) % 83) & (1 << 4)) ? 0x21 : 0xde;
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

static void Experiment_MRAM_Fill() {
    log_debug("MRAM data fill");

    for (uint64_t i = 0; i <= MRAM_max_address; i += 8) {
        uint64_t value = 0;
        for (uint64_t j = 0; j < 8; j++) {
            uint8_t value_8bit = MRAM_value((i + j) & ~mask);
            value |= ((uint64_t) (value_8bit)) << (8 * j);
        }

        *((volatile uint64_t *) (0x60000000 + i)) = value;

        MRAM_progress_report('f', i, MRAM_max_address);
    }

    MRAM_progress_report('f', MRAM_max_address, MRAM_max_address);
}

void Experiment_MRAM_Start() {
    HAL_SRAM_MspInit(&hsram1);
}

void Experiment_MRAM_Stop() {
    HAL_SRAM_MspDeInit(&hsram1);
}

void Experiment_MRAM_Loop() {
    MRAM_progress_report('r', 0, MRAM_max_address);

    Experiment_MRAM_Fill();
    log_debug("MRAM read loop: start");

    uint32_t errors = 0;

    uint32_t start_time = HAL_GetTick();
    uint32_t stop_time = HAL_GetTick();
    for (uint32_t i = 0; i <= MRAM_max_address; i += 1) {
        if ((i & mask) != 0) continue;

        uint8_t expected_value = MRAM_value(i);
        uint8_t read = MRAM_read(i);

        // STEP 1: Read old value
        if (read != expected_value) {
            errors += 1;

            // Second read to determine if the event is a transient or not
            bool isTransient = false;

            volatile uint8_t read_second = MRAM_read(i);
            if (read_second == expected_value) {
                isTransient = true;
            }

            log_warn("Error  %c [%07x] %d -> %d %d", isTransient ? 'T' : 'U', i, (int) expected_value,
                     (int) read,
                     (int) read_second
            );
        }

        // STEP 2: Write + read new value
        uint8_t new_value = ~expected_value;
        MRAM_write(i, new_value);
        uint8_t new_read = MRAM_read(i);
        if (new_read != new_value) {
            errors += 1;

            bool isTransient = false;
//
            uint8_t read_second = MRAM_read(i);
            if (read_second == new_value) {
                isTransient = true;
            }

            log_warn("Error 2%c [%07x] %d -> %d %d", isTransient ? 'T' : 'U', i, (int) expected_value,
                     (int) read,
                     (int) read_second
            );
        }

        MRAM_progress_report('r', i, MRAM_max_address);
    }

    MRAM_progress_report('r', MRAM_max_address, MRAM_max_address);

    stop_time = HAL_GetTick();

    log_info("MRAM read loop: stop %ld/%ld errors [%ld]", errors, (2 * (MRAM_max_address + 1)) / (1 << __builtin_popcount(mask)), stop_time - start_time);
}

