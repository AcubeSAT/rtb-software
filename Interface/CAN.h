#ifndef RADIATIONINTERFACE_CAN_H
#define RADIATIONINTERFACE_CAN_H

#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>


class CAN {
public:
    struct Stats {
        uint64_t rxBytes;
        uint64_t rxPackets;
        uint64_t txBytes;
        uint64_t txPackets;
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
            SEL,
            SET,
            MBU
        };

        MeasuredType measuredType;
        GuessedType guessedType;
        uint32_t flips;
        Data rx;
        Data tx;
        std::string info;

        std::string toBits(Data number) const;
        ImColor colour() const;

        std::string computerTime;
        std::string mcuTime;
    };
private:
    std::mutex timeLogMutex;
    std::vector<Event> timeLog;

    inline static const float pad = 5.0f;

    static ImVec2 padMin(ImVec2 in) {
        in.x -= pad;
        in.y -= pad;
        return in;
    }
    static ImVec2 padMax(ImVec2 in) {
        in.x += pad;
        in.y += pad;
        return in;
    }

    std::atomic<Stats> stats{};
public:
    CAN() {
        stats = {0, 0, 0, 0};
    };

    void setStats(const Stats& stats) {
        this->stats.store(stats);
    }

    void window();

    void logEvent(Event::Data rx, Event::Data tx, Event::MeasuredType = Event::BitFlip, std::string info = "");
};


#endif //RADIATIONINTERFACE_CAN_H
