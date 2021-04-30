#include <magic_enum.hpp>
#include "MRAM.h"
#include "main.h"
#include "Utilities.h"

void MRAM::window() {
    auto fillWidth = ImGui::GetContentRegionAvail().x / 4;

    ImGui::ProgressBar(progressFill.first / (float) progressFill.second, ImVec2(fillWidth, 0));
    ImGui::SameLine();
    ImGui::Text("fill");
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (ImVec4) ImColor::HSV(0.52f, 1.0f, 0.9f));
    ImGui::ProgressBar(progressRead.first / (float) progressRead.second, ImVec2(fillWidth, 0));
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Text("read");
    ImGui::SameLine(0, 40);
    ImGui::Text("Loops completed: %ld", 100);

    {
        const std::lock_guard lock(timeLogMutex);

        if (!timeLog.empty()) {
            ImGui::Separator();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            auto lastItem = timeLog.cend() - 1;

            ImGui::Text("TX:");
            ImGui::SameLine(50.0f, 0.0f);
            ImGui::PushFont(logFont);
            ImGui::Text("%#018lx", 0);
            ImGui::PopFont();
            draw_list->AddRectFilled(padMin(ImGui::GetItemRectMin()), padMax(ImGui::GetItemRectMax()), IM_COL32(75, 75, 75, 120));

            ImGui::SameLine(200.0f, 0.0f);
            ImGui::PushFont(logFont);
            ImGui::Text("%s", 0);
            ImGui::PopFont();
            draw_list->AddRectFilled(padMin(ImGui::GetItemRectMin()), padMax(ImGui::GetItemRectMax()), IM_COL32(75, 75, 75, 120));

            float currentPosition = ImGui::GetCursorPosY();

            ImGui::PushFont(largeFont);
            ImGui::SameLine(0.0f, 50.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.59, 0.2f, 0.4f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.59, 0.7f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.59, 0.7f, 0.5f, 1.0f));
            ImGui::Button(std::to_string(timeLog.size()).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 50.0f));
            ImGui::PopStyleColor(3);
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Text("errors");
            ImGui::SetCursorPosY(currentPosition);

            ImGui::Spacing();

            ImGui::Text("RX:");
            ImGui::SameLine(50.0f, 0.0f);
            ImGui::PushFont(logFont);
            ImGui::Text("%#018lx", 0);
            ImGui::PopFont();
            draw_list->AddRectFilled(padMin(ImGui::GetItemRectMin()), padMax(ImGui::GetItemRectMax()), IM_COL32(75, 75, 75, 120));

            ImGui::SameLine(200.0f, 0.0f);
            ImGui::PushFont(logFont);
            ImGui::Text("%s", 0);
            ImGui::PopFont();
            draw_list->AddRectFilled(padMin(ImGui::GetItemRectMin()), padMax(ImGui::GetItemRectMax()), IM_COL32(75, 75, 75, 120));
        }
    }

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("MRAM error log");
    ImGui::Spacing();
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("table_mram_timelog", 6, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make header row always visible
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30);
        ImGui::TableSetupColumn("Experiment Time", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("#flips", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("Error Code", ImGuiTableColumnFlags_WidthStretch, 100);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(timeLog.size());
        while (clipper.Step()) {
            const std::lock_guard lock(timeLogMutex);
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                int index = timeLog.size() - row - 1;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Selectable(std::to_string(index + 1).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", timeLog[index].experimentTime.c_str());

                ImGui::TableSetColumnIndex(2);
                std::string state(magic_enum::enum_name(timeLog[index].state));
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", state.c_str());

                ImGui::TableSetColumnIndex(3);
                std::string guessedType(magic_enum::enum_name(timeLog[index].guessedType));
//                ImGui::TextColored(timeLog[index].colour(), "%s", guessedType.c_str());

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", timeLog[index].flips);

                ImGui::TableSetColumnIndex(5);
                ImGui::Text("wat");
            }
        }
        ImGui::EndTable();
    }
}
