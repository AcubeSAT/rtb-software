#ifndef RADIATIONINTERFACE_SETTINGS_H
#define RADIATIONINTERFACE_SETTINGS_H

#include <array>
#include "cereal/cereal.hpp"
#include "cereal/types/array.hpp"
#include <fstream>

class Settings {
    inline static const std::string filename = "settings.json";
    std::ofstream output;
public:
    Settings();

    ~Settings();

    std::array<std::string, 6> logShortcuts{"A", "B", "C", "D", "E", "F"};

    void flush();

    template<class Archive>
    void serialize(Archive &archive) {
        archive(
            CEREAL_NVP(logShortcuts)
        );
    }
};

#endif //RADIATIONINTERFACE_SETTINGS_H
