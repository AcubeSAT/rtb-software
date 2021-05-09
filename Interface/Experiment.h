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

    enum class ExperimentCommand {
        None,
        Pause
    };
private:
    Duration previousDuration = 0s;
    Duration downtimeDuration = 0s;

    std::optional<TimePoint> startTime;
    std::optional<TimePoint> stopTime;
    bool underDowntime = false;

    static int currentExperimentId;
    static std::reference_wrapper<Experiment> currentExperiment;

    Status status = Idle;

    inline static std::atomic<ExperimentCommand> experimentCommand = ExperimentCommand::None;

    Duration downtime() {
        if (status != Idle) {
            if (underDowntime) {
                auto now = std::chrono::steady_clock::now();
                return downtimeDuration + (now - startTime.value());
            } else {
                return downtimeDuration;
            }
        } else {
            return 0s;
        }
    }

    Duration onDuration() {
        return duration() - downtime();
    }

    void dumpTimes();
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

    static void sendExperimentCommand(ExperimentCommand experimentCommand) {
        Experiment::experimentCommand = experimentCommand;
    }

    static Duration getCurrentExperimentDuration() {
        return currentExperiment.get().duration();
    }

    static void modifyCurrentExperimentDowntime(bool isDown) {
        if (isDown) {
            currentExperiment.get().startDowntime();
        } else {
            currentExperiment.get().stopDowntime();
        }
    }

    Duration duration() {
        if (status == Started && !underDowntime) {
            auto now = std::chrono::steady_clock::now();
            return previousDuration + now - startTime.value();
        } else {
            return previousDuration;
        }
    }

    void startDowntime() {
        if (status == Started) {
            stopTime = std::chrono::steady_clock::now();
            auto duration = stopTime.value() - startTime.value();
            previousDuration += duration;

            underDowntime = true;
            startTime = std::chrono::steady_clock::now();
        }
    }

    void stopDowntime() {
        if (status == Started) {
            stopTime = std::chrono::steady_clock::now();
            auto duration = stopTime.value() - startTime.value();
            downtimeDuration += duration;

            underDowntime = false;
            startTime = std::chrono::steady_clock::now();
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
