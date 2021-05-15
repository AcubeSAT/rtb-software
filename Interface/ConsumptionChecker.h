#ifndef RADIATIONINTERFACE_CONSUMPTIONCHECKER_H
#define RADIATIONINTERFACE_CONSUMPTIONCHECKER_H

#include <functional>
#include <optional>
#include <chrono>
#include <string>

class ConsumptionChecker {
    clock_t lastCPU, lastSysCPU, lastUserCPU;
public:
    ConsumptionChecker();

    double cpu();
    std::string memory();
    bool isBatteryDischarging();
};

template<typename V, class R, class P>
class Throttled {
    const std::chrono::duration<R, P> period;

    std::function<const V()> function;
    V lastValue;
    std::chrono::steady_clock::time_point lastAcquisition;

public:
    explicit Throttled(std::chrono::duration<R, P> period, const std::function<V()> &function) : function(function), period(period) {}

    V operator()() {
        if (std::chrono::steady_clock::now() - lastAcquisition> period) {
            lastValue = function();
            lastAcquisition = std::chrono::steady_clock::now();
        }

        return lastValue;
    }
};

// Template deduction guide
template<typename V, class R, class P>
Throttled(std::chrono::duration<R, P> & period, const std::function<V()> &) -> Throttled<V, R, P>;



#endif //RADIATIONINTERFACE_CONSUMPTIONCHECKER_H
