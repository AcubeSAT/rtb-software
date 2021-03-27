#ifndef RADIATIONINTERFACE_MEASUREMENT_H
#define RADIATIONINTERFACE_MEASUREMENT_H

#include <string>
#include <vector>
#include <mutex>

class Measurement {
    typedef float TimePoint;
    typedef float Value;

    std::vector<std::pair<std::vector<TimePoint>,std::vector<Value>>> measurements {
        std::make_pair(std::vector<TimePoint>(), std::vector<Value>())
    };

    std::mutex measurementMutex;

    void clear();
public:
    void window();

    void acquire(const std::vector<float>& values);
    void acquire(int index, float value);
};


#endif //RADIATIONINTERFACE_MEASUREMENT_H
