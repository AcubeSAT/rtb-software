#ifndef RADIATIONINTERFACE_LATCHUPS_H
#define RADIATIONINTERFACE_LATCHUPS_H

#include <boost/circular_buffer.hpp>
#include <atomic>
#include <mutex>

class Latchups {
    std::atomic_int latchupCounter = 0;

    int newLatchupCounter = 0;

    void setPopup();
    void resetPopup();
public:
    struct LatchupEvent {
        std::string computerTime;
        std::string mcuTime;
        std::string experimentTime;
        double unixTime;
        std::string state = "";
        float thresholdAtLatchup = 0;
    };

    std::mutex timeLogMutex;
    std::vector<LatchupEvent> timeLog;

    void window();

    void logLatchup(const std::string & state = "");

    void resetCounter() {
        latchupCounter = 0;
    }

    void reset() {
        const std::lock_guard lock(timeLogMutex);
        resetCounter();
        timeLog.clear();
    }

    void setCounter(int value) {
        latchupCounter = value;
    }

    const std::vector<LatchupEvent>& getAllLatchups() {
        const std::lock_guard lock(timeLogMutex);
        return timeLog;
    }
};


#endif //RADIATIONINTERFACE_LATCHUPS_H
