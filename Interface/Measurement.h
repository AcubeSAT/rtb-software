#ifndef RADIATIONINTERFACE_MEASUREMENT_H
#define RADIATIONINTERFACE_MEASUREMENT_H

#include <string>
#include <vector>
#include <mutex>

class Measurement {
public:
    static constexpr size_t SIZE = 4;
private:

    typedef float TimePoint;
    typedef float Value;

    std::array<std::pair<std::vector<TimePoint>,std::vector<Value>>, SIZE> measurements {
        std::make_pair(std::vector<TimePoint>(), std::vector<Value>())
    };

    std::mutex measurementMutex;

    void clear();
public:
    void window();

    void acquire(const std::array<float, SIZE>& value);

    bool getLCLStatus() {
        const auto& measurement = measurements[2];
        if (measurement.second.empty()) {
            return false;
        }
        return measurement.second.back();
    }
};


#endif //RADIATIONINTERFACE_MEASUREMENT_H
