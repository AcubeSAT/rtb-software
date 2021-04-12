#ifndef RADIATIONINTERFACE_MAIN_H
#define RADIATIONINTERFACE_MAIN_H

#include "SerialHandler.h"
#include "Latchups.h"
#include "Measurement.h"
#include "CAN.h"
#include "Settings.h"
#include "Beep.h"
#include "CSV.h"

const uint8_t UART_CONTROL = '\x11';

extern std::unique_ptr<SerialHandler> serialHandler;
extern Latchups latchups;
extern Measurement measurements;
extern CAN can;
extern std::optional<Beep> beep;
extern std::optional<CSV> csv;

extern ImFont* largeFont;
extern ImFont* veryLargeFont;
extern ImFont* logFont;
extern ImFont* iconFont;

extern std::optional<plog::RollingFileAppender<plog::TxtFormatter, plog::NativeEOLConverter<>>> logFileAppender;
extern std::optional<Settings> settings;

void HelpTooltip(const std::string& text);
void HelpMarker(const std::string& text);

#endif //RADIATIONINTERFACE_MAIN_H
