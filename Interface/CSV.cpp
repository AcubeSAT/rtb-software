#include "CSV.h"
#include "Log.h"
#include <plog/Log.h>
#include <boost/algorithm/string/join.hpp>

CSV::CSV()  {
    createFile("measurements", {
        "time",
        "mcuTime",
        "experimentTime",
        "value0",
        "value1"
    });
}

void CSV::createFile(const std::string & filename, const std::vector<std::string> & columns) {
    if (files.find(filename) == files.end()) {
        files[filename] = std::make_pair(std::ofstream(LogControl::getLogFileName(filename), std::ios::app), std::chrono::steady_clock::now());
        // TODO: Add exceptions
        if (files[filename].first.tellp() == 0) {
            LOG_VERBOSE << "Created new log file " << filename;
        }

        files[filename].first << boost::algorithm::join(columns, ", ") << std::endl;
    }
}
