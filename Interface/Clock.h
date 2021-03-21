#ifndef RADIATIONINTERFACE_CLOCK_H
#define RADIATIONINTERFACE_CLOCK_H

#include <sstream>
#include <atomic>

extern std::atomic<std::uint32_t> microcontrollerClock;

std::stringstream currentDatetime();
std::stringstream currentDatetimeMilliseconds();
template<typename T, typename R> std::stringstream formatDuration(std::chrono::duration<T, R> ns, bool showFraction = true);

void clockWindow();



#endif //RADIATIONINTERFACE_CLOCK_H
