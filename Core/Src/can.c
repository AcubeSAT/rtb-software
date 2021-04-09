#include <memory.h>
#include <stdlib.h>
#include "can.h"
#include "main.h"
#include "log.h"

uint8_t TxData[] = {'w', 'h', 'o', ' ', 'd', 'i', 's', '?'};
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[16] = {'\0'};
FDCAN_TxHeaderTypeDef TxHeader;

// Basically a union structure
uint64_t * RxInt = (uint64_t*) RxData;
uint64_t * TxInt = (uint64_t*) TxData;

struct Stats {
    uint64_t bytesTX;
    uint64_t packetsTX;
    uint64_t bytesRX;
    uint64_t packetsRX;
} stats;

const uint32_t can_timeout = 100;

uint64_t flipRandomBit(uint64_t input, uint32_t bits) {
    for (int i = 0; i < bits; i++) {
        uint32_t bit = rand() % 64;

        input ^= 1 << bit;
    }

    return input;
}

void Experiment_CAN_Start() {
    TxHeader.Identifier = 0x111;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    HAL_FDCAN_Start(&hfdcan1);
    HAL_FDCAN_Start(&hfdcan2);
    HAL_Delay(50);

    stats.bytesTX = 0;
    stats.bytesRX = 0;
    stats.packetsRX = 0;
    stats.packetsTX = 0;
}

void Experiment_CAN_Loop() {
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, &TxData[0]) != HAL_OK) {
        log_error("CAN TX error %#010lx", hfdcan1.ErrorCode);
        printf(UART_CONTROL UART_C_CANERROR "%s %#010lx\r\n", "TXError", hfdcan1.ErrorCode);
    }
    stats.bytesTX += 8;
    stats.packetsTX++;

    uint32_t fill_level;
    volatile uint32_t time_start = HAL_GetTick();
    do {
        fill_level = HAL_FDCAN_GetRxFifoFillLevel(&hfdcan2, FDCAN_RX_FIFO0);
    } while (fill_level != 1 && HAL_GetTick() - time_start < can_timeout);

    if (fill_level != 1) {
        log_error("CAN Timeout");
        printf(UART_CONTROL UART_C_CANERROR "%s %ldms\r\n", "Timeout", HAL_GetTick() - time_start);
        HAL_FDCAN_AbortTxRequest(&hfdcan1, FDCAN_TX_BUFFER0);
    } else {
        uint32_t duration = HAL_GetTick() - time_start;
        if (HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
            // Very representative bit randomizer
            if (rand() % 17 == 0) {
                *((uint64_t *) RxData) = flipRandomBit(*((uint64_t *) RxData), (rand() % 4) + 1);
            }

            log_trace("CAN TXRX %#018llx %#018llx [%5d]", *TxInt, *RxInt, duration);

            if (*RxInt != *TxInt) {
                uint64_t diff = *RxInt ^ *TxInt;
                uint32_t flips = __builtin_popcount(diff);

                printf(UART_CONTROL UART_C_CANBITERROR "%llx %llx\r\n", *TxInt, *RxInt);
                log_error("CAN data error [%db] %#018llx %#018llx", flips, *TxInt, *RxInt);
            }
        } else {
            printf(UART_CONTROL UART_C_CANERROR "%s %#010lx\r\n", "RXError", hfdcan2.ErrorCode);
            log_error("CAN RX error %#010lx", hfdcan2.ErrorCode);
        }
    }

//    (*((uint64_t*)TxData))++; // Don't even ask

    if (TxData[0] == 0xFF) {
        memset(TxData, 0x00, 8);
    } else {
        memset(TxData, 0xFF, 8);
    }

    printf(UART_CONTROL UART_C_STATISTICS "CAN %lld %lld %lld %lld\r\n", stats.bytesTX, stats.packetsTX, stats.bytesRX, stats.packetsRX);
}

void Experiment_CAN_Stop() {
    HAL_FDCAN_Stop(&hfdcan1);
    HAL_FDCAN_Stop(&hfdcan2);
}