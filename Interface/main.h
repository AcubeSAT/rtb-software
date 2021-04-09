#ifndef RADIATIONINTERFACE_MAIN_H
#define RADIATIONINTERFACE_MAIN_H

#include "SerialHandler.h"
#include "Latchups.h"
#include "Measurement.h"
#include "CAN.h"

const uint8_t UART_CONTROL = '\x11';

extern std::unique_ptr<SerialHandler> serialHandler;
extern Latchups latchups;
extern Measurement measurements;
extern CAN can;

extern ImFont* largeFont;
extern ImFont* veryLargeFont;
extern ImFont* logFont;

#endif //RADIATIONINTERFACE_MAIN_H
