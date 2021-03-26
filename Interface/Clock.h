#ifndef RADIATIONINTERFACE_CLOCK_H
#define RADIATIONINTERFACE_CLOCK_H

#include <sstream>
#include <atomic>

extern std::atomic<std::uint32_t> microcontrollerClock;

std::stringstream getLogFileName(const std::string& extra);
std::stringstream currentDatetime(const std::string& format = "%Y-%m-%d %X");
std::stringstream currentDatetimeMilliseconds();
template<typename T, typename R> std::stringstream formatDuration(std::chrono::duration<T, R> ns, bool showFraction = true);

void clockWindow();



#endif //RADIATIONINTERFACE_CLOCK_H
