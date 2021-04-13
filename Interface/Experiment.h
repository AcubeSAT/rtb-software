#ifndef RADIATIONINTERFACE_EXPERIMENT_H
#define RADIATIONINTERFACE_EXPERIMENT_H

#include <vector>
#include <string>
#include <chrono>
#include <plog/Log.h>
#include "Clock.h"
#include "Log.h"

using namespace std::chrono_literals;

class Experiment {
public:
    typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
    typedef std::chrono::steady_clock::duration Duration;

    enum Status {
        Idle,
        Started,
        Paused
    };
private:
    Duration previousDuration = 0s;
    std::optional<TimePoint> startTime;
    std::optional<TimePoint> stopTime;

    static int currentExperimentId;
    static std::reference_wrapper<Experiment> currentExperiment;

    Status status = Idle;
public:
    std::string name;
    std::string description;

    Experiment(const std::string &name, const std::string &description) : name(name), description(description) {}

    static std::vector<Experiment> experiments;
    static void window();
    static void resetPopup();

    static Experiment getCurrentExperiment() {
        return currentExperiment;
    }

    static Duration getCurrentExperimentDuration() {
        return currentExperiment.get().duration();
    }

    Duration duration() {
        if (status == Started) {
            auto now = std::chrono::steady_clock::now();
            return previousDuration + now - startTime.value();
        } else {
            return previousDuration;
        }
    }

    void start();

    void stop();

    void reset();

    auto getStatus() {
        return status;
    }
};


#endif //RADIATIONINTERFACE_EXPERIMENT_H
