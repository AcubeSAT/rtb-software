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

    float valuesPerSecond = 1000.0f * lastStatisticsCount / (float) std::chrono::duration_cast<std::chrono::milliseconds>(statisticsPeriod).count();

    std::ostringstream ss;
    ss << "Total values: " << std::setw(10) << measurements[0].first.size();
    ss << "\t " << "Frequency: " << std::setw(5) << std::setprecision(2) << valuesPerSecond << " values/sec";
    std::string statistics = ss.str();

    ImGui::SameLine();
    ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(statistics.c_str()).x);
    ImGui::TextUnformatted(statistics.c_str());

    static ImPlotFlags plotFlags = ImPlotFlags_AntiAliased;
    static ImPlotAxisFlags xAxisFlags = ImPlotAxisFlags_Time;
    static ImPlotAxisFlags yAxisFlags = ImPlotAxisFlags_LockMin;

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

        ImVec4 col = ImPlot::GetLastItemColor();
//        ImPlot::AnnotateClamped(0.75,0.25,ImVec2(-15,15), col, "BL");
//        ImPlot::AnnotateClamped(0.25,1,ImVec2(-15,15), col,"BL");
//        ImPlot::AnnotateClamped(0.5,2,ImVec2(-15,15),col, "aaa");

        ImPlot::EndPlot();
    }
}

void Measurement::acquire(const std::array<float, SIZE>& values) {
    using namespace std::chrono;

    static auto startTime = steady_clock::now();
    static auto lastFrequencyMeasurement = steady_clock::now();

    if (microcontrollerClock != 0) { // Ignore wrong initial values
        milliseconds msStart = duration_cast< milliseconds >(
                steady_clock::now() - startTime
        );
        auto numberMillisecondsStart = msStart.count();

        const std::lock_guard lock(measurementMutex);

        auto timestamp = currentDatetimeMillisecondsUNIX().count() / 1000.0;

        for (int i = 0; i < SIZE; i++) {
            measurements[i].first.push_back(static_cast<TimePoint>(timestamp));
            measurements[i].second.push_back(values[i]);
        }

        std::vector<std::string> csvMeasurements(values.size());
        for (int i = 0; i < values.size(); i++) {
            csvMeasurements[i] = std::to_string(values[i]);
        }

        csv->addCSVentry("measurements", csvMeasurements);

        if (steady_clock::now() - lastFrequencyMeasurement > statisticsPeriod) {
            lastStatisticsCount = currentStatisticsCount;
            currentStatisticsCount = 0;
            lastFrequencyMeasurement = steady_clock::now();
        } else {
            currentStatisticsCount++;
        }
    }
}

void Measurement::clear() {
    for (auto &measurementStack : measurements) {
        measurementStack.first.clear();
        measurementStack.second.clear();
    }

    LOG_DEBUG << "Cleared measurement display";
}

