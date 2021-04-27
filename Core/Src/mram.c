#include <stdint.h>
#include <stm32h7xx_hal.h>
#include <log.h>
#include "mram.h"

const uint64_t mask = (1 << 15);
const uint32_t MRAM_max_address = 2097151;

// Source: https://github.com/csknk/fast-modular-exponentiation
__attribute__((optimize("-Ofast"))) static inline uint64_t powm_u64(uint64_t b, uint64_t e, uint64_t m)
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

__attribute__((optimize("-Ofast"))) static inline uint32_t powm_u32(uint32_t b, uint32_t e, uint32_t m)
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

__attribute__((optimize("-Ofast"))) static inline uint8_t powm_u8(uint8_t b, uint8_t e, uint8_t m)
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

__attribute__((optimize("-Ofast"))) static inline uint8_t MRAM_value(uint32_t address) {
    return ((((uint8_t) (address * 1163) + 89) % 83) & (1 << 4)) ? 0x00 : 0xFF;
}

static inline uint8_t MRAM_read(uint32_t address) {
    return *((volatile uint8_t *) (0x60000000 + address));
}

static inline void MRAM_write(uint32_t address, uint8_t data) {
    *((volatile uint8_t *) (0x60000000 + address)) = data;
}

static void Experiment_MRAM_Fill() {
    log_trace("Filling MRAM with data");

    for (uint64_t i = 0; i <= MRAM_max_address; i += 8) {
        if ((i & mask) != 0) continue;

        uint64_t value = 0;
        for (uint64_t j = 0; j < 8; j++) {
            uint8_t value_8bit = MRAM_value(i + j);
            value |= ((uint64_t) (value_8bit)) << (8 * j);
        }

        *((volatile uint64_t *) (0x60000000 + i)) = value;
    }
}

void Experiment_MRAM_Start() {

}

void Experiment_MRAM_Loop() {
    Experiment_MRAM_Fill();

    uint32_t errors = 0;

    uint32_t start_time = HAL_GetTick();
    uint32_t stop_time = HAL_GetTick();
    for (uint64_t i = 0; i <= MRAM_max_address; i += 1) {
        if ((i & mask) != 0) continue;

        uint8_t expected_value = MRAM_value((i & (~mask)));
        uint8_t read = MRAM_read(i);

        if (read != expected_value) {
//              log_warn("Error at address %x %lld %lld %lld", i, read, *((volatile uint8_t*)((0x60000000 + i) & (~mask))), *((volatile uint8_t*)((0x60000000 + i) & (~mask))));
            errors += 1;
        }

        if (HAL_GetTick() - stop_time > 50 || i == 0 || i == MRAM_max_address) {
            log_trace("R Progress: %f", 100.0f * i / (float) MRAM_max_address);
            stop_time = HAL_GetTick();
        }
    }

    stop_time = HAL_GetTick();

    log_warn("There are %ld/%ld errors [%ld]", errors, MRAM_max_address + 1, stop_time - start_time);
}

