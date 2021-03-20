#include <imgui.h>
#include <functional>
#include "Experiment.h"
#include "Clock.h"
#import <ratio>

std::string loremIpsum = "Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here...";

std::vector<Experiment> Experiment::experiments = {
        Experiment("CAN transceivers", "An awesome CAN transceiver check"),
        Experiment("CAN transceivers silent", loremIpsum),
        Experiment("MRAM static", loremIpsum),
        Experiment("MRAM dynamic", loremIpsum),
        Experiment("MRAM check 0", loremIpsum),
        Experiment("NAND Flash", loremIpsum),
        Experiment("N-MOSFET", loremIpsum),
        Experiment("P-MOSFET", loremIpsum),
        Experiment("555 timer", loremIpsum),
        Experiment("NOR gate", loremIpsum),
        Experiment("S-R latch", loremIpsum),
        Experiment("Op-Amp", loremIpsum),
        Experiment("Shift Register", loremIpsum),
};

class num;

void Experiment::window() {
    static int currentExperimentId = 0;
    static auto currentExperiment = std::ref(experiments[0]);

    // Custom size: use all width, 5 items tall
    if (ImGui::BeginListBox("Experiments", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
        for (int n = 0; n < experiments.size(); n++) {
            const bool is_selected = (currentExperimentId == n);
            if (ImGui::Selectable(experiments[n].name.c_str(), is_selected)) {
                currentExperimentId = n;
                currentExperiment = std::ref(experiments[n]);
            }

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
    if (ImGui::Button("STOP", ImVec2(ImGui::GetContentRegionAvail().x, 100.0f))) {
        currentExperiment.get().stop();
    };
    ImGui::PopStyleColor(3);

    static float flux = 1.e10f;
    ImGui::Text("Flux:");
    ImGui::InputFloat("/cm²/s", &flux, 0.0f, 0.0f, "%e");

    ImGui::Spacing();
    ImGui::TextWrapped("%s", currentExperiment.get().description.c_str());

    if (currentExperiment.get().status == Started) {
        auto time = currentExperiment.get().duration();

        ImGui::Separator();
        ImGui::Text("Elapsed time: %s", formatDuration(time).str().c_str());

        ImGui::Text("Very Accurate Fluence Calculator:");
        float fluence = flux * std::chrono::duration_cast<std::chrono::duration<int32_t, std::ratio<1,10>>>(time).count();
        ImGui::SameLine();
        ImGui::Text("%e /cm²", fluence);
    }
}
