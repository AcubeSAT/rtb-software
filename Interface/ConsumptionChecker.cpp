#include "ConsumptionChecker.h"
#include <plog/Log.h>
#include <fstream>
#include <string>

#include "sys/times.h"
#include "sys/vtimes.h"

ConsumptionChecker::ConsumptionChecker() {
    tms timeSample;
    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;
}

std::string ConsumptionChecker::memory() {
    using namespace std::literals::chrono_literals;

    static Throttled throttled(3s, std::function([] () -> std::string {
        try {
            std::ifstream file("/proc/self/status");

            for (std::string line; std::getline(file, line);) {
                if (line.substr(0, 6) == "VmRSS:") {
                    int memoryInKiB = std::stoi(line.substr(6));
                    float memoryInMiB = memoryInKiB / 1024.0f;
                    if (memoryInMiB > 1024) {
                        return std::to_string(static_cast<int>(memoryInMiB / 1024)) + " GiB";
                    } else {
                        return std::to_string(static_cast<int>(memoryInMiB)) + " MiB";
                    }
                }
            }
        } catch (const std::exception& e) {
            LOG_WARNING << "Could not get memory consumption: " << e.what();
        }

        return "Unknown";
    }));

    return throttled();
}

double ConsumptionChecker::cpu() {
    using namespace std::literals::chrono_literals;

    static Throttled throttled(3s, std::function([this] () -> double {
        tms timeSample;
        clock_t now;
        double percent;

        now = times(&timeSample);
        if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
            timeSample.tms_utime < lastUserCPU){
            //Overflow detection. Just skip this value.
            percent = -1.0;
        }
        else{
            percent = (timeSample.tms_stime - lastSysCPU) + (timeSample.tms_utime - lastUserCPU);
            percent /= now - lastCPU;
        }
        lastCPU = now;
        lastSysCPU = timeSample.tms_stime;
        lastUserCPU = timeSample.tms_utime;

        return percent;
    }));

    return throttled();
}
