#ifndef RADIATIONINTERFACE_EXPERIMENT_H
#define RADIATIONINTERFACE_EXPERIMENT_H

#include <vector>
#include <string>
#include <chrono>

class Experiment {
    typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

    std::optional<TimePoint> startTime;
    std::optional<TimePoint> stopTime;

    enum {
        Idle,
        Started,
        Stopped
    } status;

    std::chrono::duration<double> duration() {
        using namespace std::chrono;
        if (startTime.has_value()) {
            auto now = std::chrono::steady_clock::now();
            return now - startTime.value();
        } else {
            return 0s;
        }
    }
public:
    std::string name;
    std::string description;

    Experiment(const std::string &name, const std::string &description) : name(name), description(description) {}

    static std::vector<Experiment> experiments;
    static void window();

    void start() {
        status = Started;
        startTime = std::chrono::steady_clock::now();
    }

    void stop() {
        status = Stopped;
    }
};


#endif //RADIATIONINTERFACE_EXPERIMENT_H
