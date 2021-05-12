#include "PowerCycles.h"
#include "Clock.h"
#include "main.h"

void PowerCycles::logPowerCycle(const std::string &state) {
    {
        const std::lock_guard lock(timeLogMutex);
        timeLog.push_back(Event {
                currentDatetimeMilliseconds().str(),
                formatDuration(std::chrono::milliseconds(microcontrollerClock.load())).str(),
                currentExperimentTime().str(),
                currentDatetimeMillisecondsUNIX().count() / 1000.0,
                state,
        });
    }

    csv->addCSVentry("powercycle", std::vector<std::string>{
            state
    });
}
