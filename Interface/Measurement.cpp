#include "Measurement.h"
#include "Clock.h"
#include "main.h"
#include <plog/Log.h>
#include <implot/implot.h>
#include <cmath>
#include <chrono>

void Measurement::window() {
    // Do not render the window if it is collapsed, to massively improve performance
    if (ImGui::IsWindowCollapsed()) return;

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

    static ImPlotFlags plotFlags = ImPlotFlags_AntiAliased;
    static ImPlotAxisFlags xAxisFlags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yAxisFlags = xAxisFlags | ImPlotAxisFlags_LockMin;

    if (!measurements[0].first.empty()) {
        ImPlot::SetNextPlotLimitsX(measurements[0].first.front(), measurements[0].first.back(), ImGuiCond_Always);
        ImPlot::SetNextPlotLimitsY(0, 4, ImGuiCond_Always);
    }
    if (ImPlot::BeginPlot("Measurements", "t (ms)", nullptr, ImVec2(-1, -1), plotFlags, xAxisFlags, yAxisFlags)) {
        const std::lock_guard lock(measurementMutex);
        ImPlot::PlotLine("LCL Current Sense", measurements[0].first.data(), measurements[0].second.data(), measurements[0].first.size());
        ImPlot::PlotLine("ADC 1", measurements[1].first.data(), measurements[1].second.data(), measurements[1].first.size());
        ImPlot::PlotLine("ADC 2", measurements[2].first.data(), measurements[2].second.data(), measurements[2].first.size());
        ImPlot::PlotLine("Output ON/OFF", measurements[3].first.data(), measurements[3].second.data(), measurements[3].first.size());
        ImPlot::EndPlot();
    }
}

void Measurement::acquire(const std::array<float, SIZE>& values) {
    using namespace std::chrono;

    static auto startTime = system_clock::now();

    if (microcontrollerClock != 0) { // Ignore wrong initial values
        milliseconds ms = duration_cast< milliseconds >(
                system_clock::now() - startTime
        );
        auto numberMilliseconds = ms.count();

        const std::lock_guard lock(measurementMutex);

        for (int i = 0; i < SIZE; i++) {
            measurements[i].first.push_back(numberMilliseconds);
            measurements[i].second.push_back(values[i]);
        }

        std::vector<std::string> csvMeasurements(values.size());
        for (int i = 0; i < values.size(); i++) {
            csvMeasurements[i] = std::to_string(values[i]);
        }

        csv->addCSVentry("measurements", csvMeasurements);
    }
}

void Measurement::clear() {
    for (auto &measurementStack : measurements) {
        measurementStack.first.clear();
        measurementStack.second.clear();
    }

    LOG_DEBUG << "Cleared measurement display";
}

