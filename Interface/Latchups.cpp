#include <imgui.h>
#include <plog/Log.h>
#include "Latchups.h"
#include "main.h"
#include "Clock.h"

void Latchups::window() {
    float counterHeight = std::clamp<float>(ImGui::GetContentRegionAvail().y * 0.45f, 100, 200);

    ImGui::BeginGroup();
    float currentPosition = ImGui::GetCursorPosY();
    ImGui::SetCursorPosY(currentPosition + (counterHeight - ImGui::CalcTextSize("SEL counter:").y) / 2.0f);
    ImGui::Text("SEL counter:");

    ImGui::PushFont(veryLargeFont);
    ImGui::SameLine();
    ImGui::SetCursorPosY(currentPosition);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.59, 0.7f, 0.4f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.59, 0.7f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.59, 0.7f, 0.5f, 1.0f));
    ImGui::Button(std::to_string(latchupCounter).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, counterHeight));
    ImGui::PopStyleColor(3);
    ImGui::PopFont();
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::InputInt("edit value", &newLatchupCounter);

    if (ImGui::Button("Edit", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 0))) {
        ImGui::OpenPopup("SEL Counter Set");
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.07, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.07, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.07, 0.8f, 0.8f));
    if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        ImGui::OpenPopup("SEL Counter Reset");
    }
    ImGui::PopStyleColor(3);

    ImGui::Separator();
    ImGui::Text("Latchup time log");
    ImGui::Spacing();
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("table_latchup_timelog", 3, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make header row always visible
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("MCU Time", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(timeLog.size());
        while (clipper.Step()) {
            const std::lock_guard lock(timeLogMutex);
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                int index = timeLog.size() - row - 1;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", index + 1);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", timeLog[index].computerTime.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", timeLog[index].mcuTime.c_str());
            }
        }
        ImGui::EndTable();
    }

    setPopup();
    resetPopup();
}

void Latchups::logLatchup() {
    latchupCounter++;

    {
        const std::lock_guard lock(timeLogMutex);
        timeLog.push_back(LatchupEvent {
            currentDatetimeMilliseconds().str(),
            formatDuration(std::chrono::milliseconds(microcontrollerClock.load())).str()
        });
    }
}

void Latchups::setPopup() {
    if (ImGui::BeginPopupModal("SEL Counter Set", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to set the SEL counter to %d?\n", newLatchupCounter);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            setCounter(newLatchupCounter);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void Latchups::resetPopup() {
    if (ImGui::BeginPopupModal("SEL Counter Reset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to reset the SEL counter and delete all log entries?\n");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            const std::lock_guard lock(timeLogMutex);
            resetCounter();
            timeLog.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}
