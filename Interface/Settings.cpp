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
        LOG_WARNING << "Creating new Settings file (" << e.what() << ")";
    }

    output = std::ofstream(filename);
    flush(); // Write the data once so that we don't have any issues with flushing
}

void Settings::save() {
    cereal::JSONOutputArchive archive(output);
    archive(cereal::make_nvp("settings", *this));
}

void Settings::flush() {
    output.seekp(0, std::ios::beg);
    save();
    output.flush();
}

Settings::~Settings() {
    flush();
}


