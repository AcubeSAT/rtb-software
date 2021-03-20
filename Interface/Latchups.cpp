#include <imgui.h>
#include <plog/Log.h>
#include "Latchups.h"
#include "main.h"
#include "Clock.h"

void Latchups::window() {
    float counterHeight = std::max<float>(100, ImGui::GetContentRegionAvail().y * 0.45f);

    ImGui::BeginGroup();
    float currentPosition = ImGui::GetCursorPosY();
    ImGui::SetCursorPosY(currentPosition + (counterHeight - ImGui::CalcTextSize("SEL counter:").y) / 2.0f);
    ImGui::Text("SEL counter:");

    ImGui::PushFont(largeFont);
    ImGui::SameLine();
    ImGui::SetCursorPosY(currentPosition);
    ImGui::Button(std::to_string(latchupCounter).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, counterHeight));
    ImGui::PopFont();
    ImGui::EndGroup();

    ImGui::Separator();
    ImGui::Text("Latchup time log");
    ImGui::Spacing();
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("table_latchup_timelog", 2, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make header row always visible
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(timeLog.size());
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                int index = timeLog.size() - row - 1;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", index + 1);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", timeLog[index].c_str());
            }
        }
        ImGui::EndTable();
    }
}

void Latchups::logLatchup() {
    latchupCounter++;

    {
        const std::lock_guard lock(timeLogMutex);
        timeLog.push_back(currentDatetimeMilliseconds().str());
    }
}
