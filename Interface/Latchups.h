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
    };

    std::mutex timeLogMutex;
    std::vector<LatchupEvent> timeLog;

    void window();

    void logLatchup();

    void resetCounter() {
        latchupCounter = 0;
    }

    void setCounter(int value) {
        latchupCounter = value;
    }
};


#endif //RADIATIONINTERFACE_LATCHUPS_H
