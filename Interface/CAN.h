#ifndef RADIATIONINTERFACE_CAN_H
#define RADIATIONINTERFACE_CAN_H

#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>
#include "cereal/cereal.hpp"


class CAN {
public:
    struct Stats {
        uint64_t rxBytes;
        uint64_t rxPackets;
        uint64_t txBytes;
        uint64_t txPackets;

        template<class Archive>
        void serialize(Archive &archive) {
            archive(
                CEREAL_NVP(rxBytes),
                CEREAL_NVP(rxPackets),
                CEREAL_NVP(txBytes),
                CEREAL_NVP(txPackets)
            );
        }
    };

    struct Event {
        typedef uint64_t Data;

        enum MeasuredType {
            BitFlip,
            TXError,
            RXError,
            Timeout,
            Unknown
        };

        enum GuessedType {
            SEFI,
            Hard_SEFI,
            SEL,
            SET,
            MBU
        };

        enum State {
            RX,
            TX,
            Idle
        };

        MeasuredType measuredType;
        GuessedType guessedType;
        State state;
        uint32_t flips;
        Data rx;
        Data tx;
        std::string info;

        std::string toBits(Data number) const;
        ImColor colour() const;

        std::string computerTime;
        std::string mcuTime;
        std::string experimentTime;
    };
private:
    std::mutex timeLogMutex;
    std::vector<Event> timeLog;

    std::atomic<Stats> stats{};

    int consecutiveSEFIs = 0;
public:
    CAN() {
        stats = {0, 0, 0, 0};
    };

    void setStats(const Stats& stats) {
        this->stats.store(stats);
    }

    void window();

    void reset();

    void logEvent(Event::Data rx, Event::Data tx, Event::State state, Event::MeasuredType = Event::BitFlip, const std::string& info = "");
};


#endif //RADIATIONINTERFACE_CAN_H
