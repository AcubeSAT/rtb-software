#ifndef RADIATIONINTERFACE_CAN_H
#define RADIATIONINTERFACE_CAN_H

#include <cstdint>
#include <vector>
#include <mutex>


class CAN {
public:
    struct Event {
        typedef uint64_t Data;

        enum MeasuredType {
            BitFlip,
            TXError,
            RXError
        };

        enum GuessedType {
            SEFI,
            SEL,
            SET,
            MBU
        };

        MeasuredType measuredType;
        GuessedType guessedType;
        uint32_t flips;
        Data rx;
        Data tx;

        std::string computerTime;
        std::string mcuTime;
    };
private:
    std::mutex timeLogMutex;
    std::vector<Event> timeLog;
public:
    void window();

    void logEvent(Event::Data rx, Event::Data tx, Event::MeasuredType = Event::BitFlip);
};


#endif //RADIATIONINTERFACE_CAN_H
