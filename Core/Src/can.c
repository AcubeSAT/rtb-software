#include "can.h"
#include "main.h"
#include "log.h"

uint8_t TxData[] = {'w', 'h', 'o', ' ', 'd', 'i', 's', '?'};
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[16] = {'\0'};
FDCAN_TxHeaderTypeDef TxHeader;

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
}

void Experiment_CAN_Loop() {
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, &TxData[0]) != HAL_OK) {
        log_error("I could not sent can message because %#010lx", hfdcan1.ErrorCode);
    };

    uint32_t fill_level = HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0);

    if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
        log_warn("Got new message: %#010lx", *((uint64_t*)RxData));
    }

    (*((uint64_t*)TxData))++; // Don't even ask
}

void Experiment_CAN_Stop() {
    HAL_FDCAN_Stop(&hfdcan1);
    HAL_FDCAN_Stop(&hfdcan2);
}