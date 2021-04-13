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

    flush(); // Write the data once so that we don't have any issues with flushing
}

void Settings::save() {
    cereal::JSONOutputArchive archive(output);
    archive(cereal::make_nvp("settings", *this));
}

void Settings::flush() {
    output.open(filename);
    save();
    output.close();
}

Settings::~Settings() {
    flush();
}


