#include "Settings.h"
#include <fstream>
#include <cereal/archives/json.hpp>
#include <plog/Log.h>

Settings::Settings() {
    try {
        std::ifstream is(filename);
        cereal::JSONInputArchive archive(is);

        archive(*this);
    } catch (const std::exception & e) {
        LOG_WARNING << "Creating new Settings file";
    }

    output = std::ofstream(filename, std::ios::app);
}

void Settings::flush() {
    output.flush();
    output.seekp(0, std::ios::beg);
    cereal::JSONOutputArchive archive(output);
    archive(cereal::make_nvp("settings", *this));
}

Settings::~Settings() {
    flush();
}


