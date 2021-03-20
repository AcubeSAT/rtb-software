#ifndef RADIATIONINTERFACE_LATCHUPS_H
#define RADIATIONINTERFACE_LATCHUPS_H

#include <boost/circular_buffer.hpp>
#include <atomic>

class Latchups {
    std::atomic_int latchupCounter = 0;
public:
    boost::circular_buffer<std::string> timeLog;

    void window();

    void logLatchup() {
        latchupCounter++;
    };

    void resetCounter() {
        latchupCounter = 0;
    }

    void setCounter(int value) {
        latchupCounter = value;
    }
};


#endif //RADIATIONINTERFACE_LATCHUPS_H
