#include "CSV.h"
#include "Log.h"
#include "Clock.h"
#include <plog/Log.h>
#include <boost/algorithm/string/join.hpp>

CSV::CSV()  {
    createFile("measurements", {
        "value0",
        "value1"
    });

    createFile("can", {
        "measuredType",
        "guessedType",
        "flips",
        "rx",
        "tx",
        "info"
    });

    createFile("latchup", {
    });
}

void CSV::createFile(const std::string &filename, bool force) {
    if (files.find(filename) == files.end() || force) {
        files[filename] = std::make_pair(std::ofstream(LogControl::getLogFileName(filename, "csv"), std::ios::app),
                                         std::chrono::steady_clock::now());

        if (!files[filename].first.good()) {
            LOG_FATAL << "Could not open file " << filename << " for writing";
            return;
        }

        if (files[filename].first.tellp() == 0) {
            LOG_VERBOSE << "Created new log file " << filename;
        }

        files[filename].first << "time,mcuTime,experimentTime,"
                              << boost::algorithm::join(fileSignatures[filename], ", ") << std::endl;
        files[filename].second = std::chrono::steady_clock::now();
    }
}

void CSV::createFile(const std::string & filename, const std::vector<std::string> & columns, bool force) {
    fileSignatures[filename] = columns;
    createFile(filename, force);
}

void CSV::addCSVentry(const std::string &filename, const std::vector<std::string> &data) {
    std::string timeString = boost::algorithm::join(std::array<std::string,3>{
                                                            currentDatetimeMilliseconds().str(),
                                                            std::to_string(microcontrollerClock),
                                                            currentExperimentTime().str(),
    }, ",");

    if (files.find(filename) == files.end()) {
        LOG_ERROR << "Measurement output to unprepared file " << filename;
        createFile(filename, {});
    }

    // Depending on the implementation, writing files might flush automatically or not. Having '\n' here instead of
    // std::endl prevents forced fllushing. However, we still add a predefined flushInterval to make sure that the data
    // is stored to the disc no matter what.
    files[filename].first << timeString << "," << boost::algorithm::join(data, ", ") << '\n';
    if (std::chrono::steady_clock::now() > files[filename].second + flushInterval) {
        files[filename].first.flush();
    }
}

void CSV::refreshAllFilenames() {
    for (auto& it : files) {
        if (it.second.first.is_open()) {
            it.second.first.close();
        }

        createFile(it.first, true);
    }
}
