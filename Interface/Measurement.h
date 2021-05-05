#ifndef RADIATIONINTERFACE_MEASUREMENT_H
#define RADIATIONINTERFACE_MEASUREMENT_H

#include <string>
#include <vector>
#include <mutex>

class Measurement {
public:
    static constexpr size_t SIZE = 4;
private:

    typedef double TimePoint;
    typedef double Value;

    inline static const auto statisticsPeriod = std::chrono::seconds(1);
    int currentStatisticsCount = 0;
    int lastStatisticsCount = 0;

    bool zoomable = false;
    bool tooltips = true;

    std::array<std::pair<std::vector<TimePoint>,std::vector<Value>>, SIZE> measurements {
        std::make_pair(std::vector<TimePoint>(), std::vector<Value>())
    };

    std::mutex measurementMutex;

    void clear();
public:
    void window();

    std::tuple<int, int, int> downsample();

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
