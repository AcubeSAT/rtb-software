#ifndef RADIATIONINTERFACE_SETTINGS_H
#define RADIATIONINTERFACE_SETTINGS_H

#include <array>
#include "cereal/cereal.hpp"
#include "cereal/types/array.hpp"
#include <fstream>

class Settings {
    inline static const std::string filename = "settings.json";
    std::ofstream output;

    void save();
public:
    Settings();

    ~Settings();

    std::array<std::string, 6> logShortcuts{"A", "B", "C", "D", "E", "F"};
    float volume = 0.0f;
    std::string logTitle = "Vanilla";

    void flush();

    template<class Archive>
    void serialize(Archive &archive) {
        archive(
            CEREAL_NVP(logShortcuts),
            CEREAL_NVP(volume),
            CEREAL_NVP(logTitle)
        );
    }
};

#endif //RADIATIONINTERFACE_SETTINGS_H
