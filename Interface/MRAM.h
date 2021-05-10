#ifndef RADIATIONINTERFACE_MRAM_H
#define RADIATIONINTERFACE_MRAM_H

#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>
#include <imgui.h>
#include "cereal/cereal.hpp"


class MRAM {
public:
    struct Stats {
        uint64_t bytesWritten;
        uint64_t loops;

        template<class Archive>
        void serialize(Archive &archive) {
            archive(
                    CEREAL_NVP(bytesWritten),
                    CEREAL_NVP(loops)
            );
        }
    };

    struct Event {
        typedef uint32_t Address;
        typedef uint8_t Data;

        enum GuessedType {
            SEFI,
            SEL,
            SEU,
            SET,
            MBU
        };

        enum State {
            Read,
            Write,
            Idle
        };

        GuessedType guessedType;
        State state;
        uint32_t flips;
        Address address;
        Data write;
        Data read1;

        std::string toBits(Data number) const;
        ImColor colour() const;

        std::string computerTime;
        std::string mcuTime;
        std::string experimentTime;
    };

    std::pair<int, int> progressFill;
    std::pair<int, int> progressRead;

private:
    std::mutex timeLogMutex;
    std::vector<Event> timeLog;

    std::atomic<Stats> stats{};
public:
    MRAM() {
        stats = {0, 0};
    }

    void setStats(const Stats& stats) {
        this->stats.store(stats);
    }

    void window();

    void reset();

    void logEvent(Event::Address address, Event::Data expected, Event::Data read, Event::State state);

    void setProgressFill(int progress, int max) {
        progressFill = std::make_pair(progress, max);
    }

    void setProgressRead(int progress, int max) {
        progressRead = std::make_pair(progress, max);
    }
};


#endif //RADIATIONINTERFACE_MRAM_H
