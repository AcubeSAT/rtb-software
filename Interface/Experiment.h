#ifndef RADIATIONINTERFACE_EXPERIMENT_H
#define RADIATIONINTERFACE_EXPERIMENT_H

#include <vector>
#include <string>
#include <chrono>
#include <plog/Log.h>
#include "Clock.h"

using namespace std::chrono_literals;

class Experiment {
    typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
    typedef std::chrono::steady_clock::duration Duration;

    Duration previousDuration = 0s;
    std::optional<TimePoint> startTime;
    std::optional<TimePoint> stopTime;

    enum {
        Idle,
        Started,
        Paused
    } status = Idle;

    Duration duration() {
        if (status == Started) {
            auto now = std::chrono::steady_clock::now();
            return previousDuration + now - startTime.value();
        } else {
            return previousDuration;
        }
    }
public:
    std::string name;
    std::string description;

    Experiment(const std::string &name, const std::string &description) : name(name), description(description) {}

    static std::vector<Experiment> experiments;
    static void window();

    void start() {
        if (status == Started) {
            LOG_ERROR << "The experiment has already started? It cannot be started again.";
            return;
        }

        status = Started;
        startTime = std::chrono::steady_clock::now();

        LOG_INFO << "Started experiment " << name;
    }

    void stop() {
        if (status != Started) {
            LOG_ERROR << "You can't stop an experiment if it's already stopped.";
            return;
        }

        status = Paused;
        stopTime = std::chrono::steady_clock::now();

        auto duration = stopTime.value() - startTime.value();
        previousDuration += duration;

        LOG_INFO << "Ended experiment " << name << " at " << formatDuration(duration).str();
    }

    void reset() {
        if (status == Started) {
            LOG_ERROR << "Please stop this experiment before resetting it";
            return;
        }

        LOG_INFO << "Reset experiment " << name << " at total " << formatDuration(previousDuration).str();

        status = Idle;
        previousDuration = 0s;
        startTime.reset();
        stopTime.reset();
    }

    auto getStatus() {
        return status;
    }
};


#endif //RADIATIONINTERFACE_EXPERIMENT_H
