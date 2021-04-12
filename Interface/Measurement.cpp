#include "Measurement.h"
#include "Clock.h"
#include "main.h"
#include <plog/Log.h>
#include <implot/implot.h>
#include <cmath>
#include <chrono>

void Measurement::window() {
    ImGui::Text("Last values: ");
    for (auto &measurementStack : measurements) {
        if (!measurementStack.second.empty()) {
            ImGui::SameLine();
            ImGui::SmallButton(std::to_string(measurementStack.second.back()).c_str());
        }
    }
    ImGui::SameLine(0.0f, 40.0f);
    if (ImGui::SmallButton("Reset")) {
        clear();
    }


    static ImPlotFlags plotFlags = 0;
    static ImPlotAxisFlags xAxisFlags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yAxisFlags = xAxisFlags | ImPlotAxisFlags_LockMin;

    if (!measurements[0].first.empty()) {
        ImPlot::SetNextPlotLimitsX(measurements[0].first.front(), measurements[0].first.back(), ImGuiCond_Always);
        ImPlot::SetNextPlotLimitsY(0, 4096, ImGuiCond_Always);
    }
    if (ImPlot::BeginPlot("Measurements", "t (ms)", nullptr, ImVec2(-1, -1), plotFlags, xAxisFlags, yAxisFlags)) {
        const std::lock_guard lock(measurementMutex);
        ImPlot::PlotLine("Data 1", measurements[0].first.data(), measurements[0].second.data(), measurements[0].first.size());
        ImPlot::EndPlot();
    }
}

void Measurement::acquire(int index, float value) {
    using namespace std::chrono;

    static auto startTime = system_clock::now();

    if (microcontrollerClock != 0) { // Ignore wrong initial values
        milliseconds ms = duration_cast< milliseconds >(
                system_clock::now() - startTime
        );

        const std::lock_guard lock(measurementMutex);

        measurements[index].first.push_back(ms.count());
        measurements[index].second.push_back(value);

        csv->addCSVentry("measurements", {
            currentDatetimeMilliseconds().str(),
            std::to_string(microcontrollerClock),
            currentExperimentTime().str(),
            std::to_string(value),
            "0"
        });
    }
}

void Measurement::clear() {
    for (auto &measurementStack : measurements) {
        measurementStack.first.clear();
        measurementStack.second.clear();
    }

    LOG_DEBUG << "Cleared measurement display";
}

