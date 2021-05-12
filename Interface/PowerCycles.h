#ifndef RADIATIONINTERFACE_POWERCYCLES_H
#define RADIATIONINTERFACE_POWERCYCLES_H

#include <boost/circular_buffer.hpp>
#include <atomic>
#include <mutex>

class PowerCycles {
public:
    struct Event {
        std::string computerTime;
        std::string mcuTime;
        std::string experimentTime;
        double unixTime;
        std::string state = "";
    };

    std::mutex timeLogMutex;
    std::vector<Event> timeLog;

    void logPowerCycle(const std::string & state = "");

    void reset() {
        const std::lock_guard lock(timeLogMutex);
        timeLog.clear();
    }

    const std::vector<Event>& getAllPowerCycles() {
        const std::lock_guard lock(timeLogMutex);
        return timeLog;
    }
};


#endif //RADIATIONINTERFACE_POWERCYCLES_H
