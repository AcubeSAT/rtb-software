#ifndef RADIATIONINTERFACE_MAIN_H
#define RADIATIONINTERFACE_MAIN_H

#include "SerialHandler.h"
#include "Latchups.h"

const uint8_t UART_CONTROL = '\x11';

extern std::unique_ptr<SerialHandler> serialHandler;
extern Latchups latchups;

extern ImFont* largeFont;
extern ImFont* veryLargeFont;

#endif //RADIATIONINTERFACE_MAIN_H
