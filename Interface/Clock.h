#ifndef RADIATIONINTERFACE_CLOCK_H
#define RADIATIONINTERFACE_CLOCK_H

#include <sstream>
#include <atomic>
#include <chrono>

extern std::atomic<std::uint32_t> microcontrollerClock;

std::stringstream currentDatetime(const std::string& format = "%Y-%m-%d %X");
inline std::chrono::milliseconds currentDatetimeMillisecondsUNIX() {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch());

    return ms;
}
std::stringstream currentDatetimeMilliseconds();
template<typename T, typename R> std::stringstream formatDuration(std::chrono::duration<T, R> ns, bool showFraction = true);
std::stringstream currentExperimentTime();

void clockWindow();



#endif //RADIATIONINTERFACE_CLOCK_H
