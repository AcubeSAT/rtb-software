#include "CSV.h"
#include "Log.h"
#include "Clock.h"
#include <plog/Log.h>
#include <boost/algorithm/string/join.hpp>

CSV::CSV() : fileWriterThread(&CSV::thread, this) {
    createFile("measurements", {
        "currentSense",
        "value1",
        "value2",
        "output"
    });

    createFile("can", {
        "state",
        "measuredType",
        "guessedType",
        "flips",
        "rx",
        "tx",
        "info"
    });

    createFile("mram", {
            "state",
            "guessedType",
            "address",
            "flips",
            "expected",
            "read1",
    });

    createFile("latchup", {
        "state",
        "" // This may be needed?
    });
}

void CSV::createFile(const std::string &filename, bool force) {
    const std::lock_guard<std::mutex> lock(fileMutex);

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
    std::string fileOutput = boost::algorithm::join(std::array<std::string,3>{
                                                            currentDatetimeMilliseconds().str(),
                                                            std::to_string(microcontrollerClock),
                                                            currentExperimentTime().str(),
    }, ",") + "," + boost::algorithm::join(data, ",");

    if (!queue.push(std::make_pair(filename, fileOutput))) {
        LOG_FATAL << "CSV queue has too many data to store! Consider getting a faster hard drive :)";
    } else {
        queueNotification.notify_all();
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

void CSV::thread() {
    std::unique_lock<std::mutex> lock(queueMutex);

    while (!stop) {
        queueNotification.wait(lock);

        std::pair<std::string, std::string> fileData;
        while (queue.pop(fileData)) {
            const std::lock_guard<std::mutex> lock(fileMutex);

            std::string filename = std::move(fileData.first);
            std::string data = std::move(fileData.second);

            if (files.find(filename) == files.end()) {
                LOG_ERROR << "Measurement output to unprepared file " << filename;
                createFile(filename, {});
            }

            // Depending on the implementation, writing files might flush automatically or not. Having '\n' here instead of
            // std::endl prevents forced fllushing. However, we still add a predefined flushInterval to make sure that the data
            // is stored to the disc no matter what.
            files[filename].first << data << '\n';
            if (std::chrono::steady_clock::now() > files[filename].second + flushInterval) {
                files[filename].first.flush();
                files[filename].second = std::chrono::steady_clock::now();
            }
        }
    }
}

CSV::~CSV() {
    stop = true;
    queueNotification.notify_all();
    fileWriterThread.join();
}
