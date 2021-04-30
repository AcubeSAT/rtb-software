#ifndef RADIATIONINTERFACE_MRAM_H
#define RADIATIONINTERFACE_MRAM_H

#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>
#include <imgui.h>

class MRAM {
public:
    struct Event {
        typedef uint8_t Data;

        enum GuessedType {
            SEFI,
            SEL,
            SET,
            MBU
        };

        enum State {
            Read,
            Write
        };

        GuessedType guessedType;
        State state;
        uint32_t flips;
        Data write;
        Data read1;
        Data read2;

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

public:
    void window();

    void reset();

    void logEvent();

    void setProgressFill(int progress, int max) {
        progressFill = std::make_pair(progress, max);
    }

    void setProgressRead(int progress, int max) {
        progressRead = std::make_pair(progress, max);
    }
};


#endif //RADIATIONINTERFACE_MRAM_H
