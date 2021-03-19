#include <imgui.h>
#include "Experiment.h"

void experimentWindow() {
    const char* items[] = {
            "CAN transceivers",
            "CAN transceivers silent",
            "MRAM static",
            "MRAM dynamic",
            "MRAM check 0",
            "NAND Flash",
            "N-MOSFET",
            "P-MOSFET",
            "555 timer",
            "NOR gate",
            "S-R latch",
            "Op-Amp",
            "Shift Register",
    };
    static int item_current_idx = 0; // Here we store our selection data as an index.

    // Custom size: use all width, 5 items tall
    if (ImGui::BeginListBox("Experiments", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(items[n], is_selected))
                item_current_idx = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }

    ImGui::Spacing();
    ImGui::TextWrapped("Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here... Description of the test to be placed here...");
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
    ImGui::Button("START", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 100.0f));
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
    ImGui::Button("STOP", ImVec2(ImGui::GetContentRegionAvail().x, 100.0f));
    ImGui::PopStyleColor(3);


}
