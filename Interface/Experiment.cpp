#include <imgui.h>
#include <functional>
#include "Experiment.h"
#include "Clock.h"
#include "main.h"
#include "Utilities.h"
#include <ratio>

std::string loremIpsum = "Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here...";

std::vector<Experiment> Experiment::experiments = {
        Experiment("Clear", "No experiment selected. No actions taken"),
        Experiment("CAN transceiver", "CAN RX + TX. Comparison of received messages."),
        Experiment("CAN transceiver silent", loremIpsum),
        Experiment("MRAM", "MRAM write and verify loop"),
        Experiment("555 timer", loremIpsum),
        Experiment("NOR gate", loremIpsum),
        Experiment("S-R latch", loremIpsum),
        Experiment("Op-Amp", loremIpsum),
        Experiment("Shift Register", loremIpsum),
};

int Experiment::currentExperimentId = 0;
std::reference_wrapper<Experiment> Experiment::currentExperiment = std::ref(Experiment::experiments[0]);

void Experiment::window() {
    ImGui::Text("Current Experiment:");
    ImGui::SameLine();
    HelpMarker(currentExperiment.get().description);

    const char * experimentName = currentExperiment.get().name.c_str();

    ImU32 experimentColour;
    switch (currentExperiment.get().status) {
        case Started:
            experimentColour = IM_COL32(100, 255, 110, 200);
            break;
        case Paused:
            experimentColour = IM_COL32(255, 150, 50, 200);
            break;
        default:
            experimentColour = IM_COL32(255, 255, 255, 200);
    }

    ImGui::PushFont(largeFont);
    // Place text to the middle
    ImGui::PushStyleColor(ImGuiCol_Text, experimentColour);
    ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - (ImGui::CalcTextSize(experimentName).x)
                                      - ImGui::GetScrollX()) / 2.0f
    );
    ImGui::Text("%s", experimentName);
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::Text("Current Status: ");
    ImGui::SameLine();
    switch (currentExperiment.get().status) {
        case Idle:
            ImGui::Text("Idle");
            break;
        case Started:
            ImGui::TextColored(ImColor(0.31f, 0.89f, 0.31f), "Started");
            break;
        case Paused:
            ImGui::TextColored(ImColor(0.89f, 0.31f, 0.31f), "Paused");
            break;
        default:
            ImGui::Text("????????");
    }
    ImGui::Separator();

    // Custom size: use all width, 5 items tall
    if (currentExperiment.get().status == Idle && ImGui::BeginListBox("Experiments", ImVec2(-FLT_MIN, 8 * ImGui::GetTextLineHeightWithSpacing()))) {
        const bool experimentRunning = currentExperiment.get().status != Idle;
        for (int n = 0; n < experiments.size(); n++) {
            const bool is_selected = (currentExperimentId == n);
            const auto flags = experimentRunning ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None;

            if (ImGui::Selectable(experiments[n].name.c_str(), is_selected, flags)) {
                currentExperimentId = n;
                currentExperiment = std::ref(experiments[n]);
                if (LogControl::status == LogControl::LogStatus::automatic) {
                    LogControl::reset();
                }
            }

            ImGui::SameLine();
            HelpMarker(experiments[n].description);

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
    if (ImGui::Button("START", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 100.0f))) {
        currentExperiment.get().start();
    };
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
    if (ImGui::Button("PAUSE", ImVec2(ImGui::GetContentRegionAvail().x, 100.0f))) {
        currentExperiment.get().stop();
    };
    ImGui::PopStyleColor(3);

    if (currentExperiment.get().status != Started && ImGui::Button("RESET", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
        ImGui::OpenPopup("Experiment Reset");
    };

    static float flux = 1.e10f;
    ImGui::Text("Flux:");
    ImGui::SameLine();
    HelpMarker("Set the flux to quickly calculate the total fluence for the duration of this experiment");
    ImGui::SameLine();
    ImGui::InputFloat("/cm²/s", &flux, 0.0f, 0.0f, "%.3e");

    ImGui::Spacing();

    if (currentExperiment.get().status != Idle) {
        auto time = currentExperiment.get().duration();

        ImGui::Separator();
        ImGui::Text("Elapsed time: %s", formatDuration(time).str().c_str());

        ImGui::Text("Totally Accurate Fluence Calculator:");
        float fluence = flux * std::chrono::duration_cast<std::chrono::duration<int32_t, std::ratio<1,10>>>(time).count();
        ImGui::SameLine();
        ImGui::Text("%.3e /cm²", fluence);
    }

    resetPopup();
}

void Experiment::resetPopup() {
    if (ImGui::BeginPopupModal("Experiment Reset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to reset the experiment \"%s\"? ALL timings and data from other windows will be lost.\n", currentExperiment.get().name.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            currentExperiment.get().reset();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void Experiment::start() {
    if (status == Started) {
        LOG_ERROR << "The experiment has already started? It cannot be started again.";
        return;
    }

    if (status == Idle) {
        LogControl::saveNewLogTitle();
    }

    status = Started;
    startTime = std::chrono::steady_clock::now();

    serialHandler->write("p1\n");
    serialHandler->write(std::string("et") + std::to_string(currentExperimentId) + "\n");

    LOG_INFO << "Started experiment " << name;
}

void Experiment::stop() {
    if (status != Started) {
        LOG_ERROR << "You can't stop an experiment if it's already stopped.";
        return;
    }

    serialHandler->write("p0\n");
    serialHandler->write("ep\n");

    status = Paused;
    stopTime = std::chrono::steady_clock::now();

    auto duration = stopTime.value() - startTime.value();
    previousDuration += duration;

    LOG_INFO << "Paused experiment " << name << " at " << formatDuration(duration).str();
}

void Experiment::reset() {
    if (status == Started) {
        LOG_ERROR << "Please stop this experiment before resetting it";
        return;
    }

    serialHandler->write(std::string("er") + std::to_string(currentExperimentId) + "\n");

    LOG_INFO << "Reset experiment " << name << " at total " << formatDuration(previousDuration).str();

    status = Idle;
    previousDuration = 0s;
    startTime.reset();
    stopTime.reset();

    can.reset();
    latchups.reset();
    mram.reset();
}
