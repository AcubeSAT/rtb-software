#include <memory.h>
#include <stdlib.h>
#include <experiments.h>
#include "can.h"
#include "main.h"
#include "log.h"
#include "parameters.h"

uint8_t TxData[] = {'w', 'h', 'o', ' ', 'd', 'i', 's', '?'};
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[16] = {'\0'};
FDCAN_TxHeaderTypeDef TxHeader;

static bool experimentStatus = false;

// Basically a union structure
uint64_t * RxInt = (uint64_t*) RxData;
uint64_t * TxInt = (uint64_t*) TxData;

static struct Stats {
    uint64_t bytesTX;
    uint64_t packetsTX;
    uint64_t bytesRX;
    uint64_t packetsRX;
} stats = { 0, 0, 0, 0};

void MX_FDCAN1_Init();

static FDCAN_HandleTypeDef* testedCAN ;
static FDCAN_HandleTypeDef* ambientCAN;

static FDCAN_HandleTypeDef* rxCAN;
static FDCAN_HandleTypeDef* txCAN;

const uint32_t can_timeout = 50; // Timeout in milliseconds

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

    HAL_FDCAN_MspInit(&hfdcan1);
    HAL_FDCAN_MspInit(&hfdcan2);
    HAL_FDCAN_Start(&hfdcan1);
    HAL_FDCAN_Start(&hfdcan2);
    HAL_Delay(2);

    testedCAN = &hfdcan1;
    ambientCAN = &hfdcan2;

    state = canRX;
    rxCAN = testedCAN;
    txCAN = ambientCAN;

    experimentStatus = true;
}


static void Experiment_CAN_Statistics() {
    static uint32_t lastStats = 0;

    if (HAL_GetTick() - lastStats > 100) {
        lastStats = HAL_GetTick();
        printf(UART_CONTROL UART_C_STATISTICS "CAN %lld %lld %lld %lld\r\n", stats.bytesTX, stats.packetsTX, stats.bytesRX, stats.packetsRX);
    }
}

void Experiment_CAN_Silent_Loop() {
    rxCAN = testedCAN;
    txCAN = ambientCAN;

    if (!experimentStatus) return;

    if (HAL_FDCAN_AddMessageToTxFifoQ(txCAN, &TxHeader, &TxData[0]) != HAL_OK) {
        HAL_Delay(200);
    }

    stats.bytesTX += 8;
    stats.packetsTX++;

    Experiment_CAN_Statistics();
}

void Experiment_CAN_Loop() {
    if (state == canRX) {
        rxCAN = testedCAN;
        txCAN = ambientCAN;
    } else if (state == canTX) {
        rxCAN = ambientCAN;
        txCAN = testedCAN;
    }

    static char state_string[STATE_STRING_SIZE];
    state_to_string(state, state_string);

    if (!experimentStatus) return;

    if (HAL_FDCAN_AddMessageToTxFifoQ(txCAN, &TxHeader, &TxData[0]) != HAL_OK) {
        log_error("CAN TX error %#010lx", txCAN->ErrorCode);
        printf(UART_CONTROL UART_C_CANERROR "%s %s %#010lx\r\n", "TXError", state_string, txCAN->ErrorCode);
    }

    uint32_t fill_level;
    volatile uint32_t time_start = HAL_GetTick();
    do {
        fill_level = HAL_FDCAN_GetRxFifoFillLevel(rxCAN, FDCAN_RX_FIFO0);
    } while (fill_level != 1 && HAL_GetTick() - time_start < can_timeout);

    if (fill_level != 1 && experimentStatus) {
        log_error("CAN Timeout");
        printf(UART_CONTROL UART_C_CANERROR "%s %s %ldms\r\n", "Timeout", state_string, HAL_GetTick() - time_start);
        for (uint32_t i = FDCAN_TX_BUFFER0; i <= FDCAN_TX_BUFFER31 && i != 0; i = i << 1) {
            // Maybe this is not needed since we stop the CANs
            HAL_FDCAN_AbortTxRequest(txCAN, i);
        }
        HAL_FDCAN_Stop(txCAN);
        HAL_FDCAN_Stop(rxCAN);
        HAL_FDCAN_Start(txCAN);
        HAL_FDCAN_Start(rxCAN);
    } else {
        uint32_t duration = HAL_GetTick() - time_start;
        if (HAL_FDCAN_GetRxMessage(rxCAN, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
            // Very representative bit randomizer
            if (RANDOM_ERRORS && rand() % 511 == 0) {
                *((uint64_t *) RxData) = flipRandomBit(*((uint64_t *) RxData), (rand() % 4) + 1);
            }

//            log_trace("CAN TXRX %#018llx %#018llx [%5d]", *TxInt, *RxInt, duration);

            if (*RxInt != *TxInt && experimentStatus) {
                uint64_t diff = *RxInt ^ *TxInt;
                uint32_t flips = __builtin_popcount(diff);

                printf(UART_CONTROL UART_C_CANBITERROR "%s %llx %llx\r\n", state_string, *TxInt, *RxInt);
                log_error("CAN data error [%db] %#018llx %#018llx", flips, *TxInt, *RxInt);
            }
        } else if (experimentStatus) {
            printf(UART_CONTROL UART_C_CANERROR "%s %s %#010lx\r\n", "RXError", state_string, rxCAN->ErrorCode);
            log_error("CAN RX error %#010lx", rxCAN->ErrorCode);
        }
    }

//    (*((uint64_t*)TxData))++; // Don't even ask

    if (TxData[0] == 0xFF) {
        memset(TxData, 0x00, 8);
    } else {
        memset(TxData, 0xFF, 8);
    }

    if (state == canRX) {
        stats.bytesRX += 8;
        stats.packetsRX++;
        state = canTX;
    } else {
        stats.bytesTX += 8;
        stats.packetsTX++;
        state = canRX;
    }

    Experiment_CAN_Statistics();
}

void Experiment_CAN_DeinitPins() {
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);
}

void Experiment_CAN_Stop() {
    Experiment_CAN_DeinitPins();

    for (uint32_t i = FDCAN_TX_BUFFER0; i <= FDCAN_TX_BUFFER31 && i != 0; i = i << 1) {
        // Maybe this is not needed since we stop the CANs
        HAL_FDCAN_AbortTxRequest(txCAN, i);
    }

    HAL_FDCAN_Stop(&hfdcan1);
    HAL_FDCAN_Stop(&hfdcan2);

    experimentStatus = false;
}

void Experiment_CAN_Reset() {
    stats.bytesTX = 0;
    stats.bytesRX = 0;
    stats.packetsRX = 0;
    stats.packetsTX = 0;
}