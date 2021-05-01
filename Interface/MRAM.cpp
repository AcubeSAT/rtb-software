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
    ImGui::Text("verify");
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
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Experiment Time", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("#flips", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch, 100);
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
                ImGui::Text( "%s", state.c_str());

                ImGui::TableSetColumnIndex(3);
                std::string guessedType(magic_enum::enum_name(timeLog[index].guessedType));
                ImGui::TextColored(timeLog[index].colour(), "%s", guessedType.c_str());

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", timeLog[index].flips);

                ImGui::TableSetColumnIndex(5);
                ImGui::PushFont(logFont);
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),"0x%07x", timeLog[index].address);
                ImGui::PopFont();
            }
        }
        ImGui::EndTable();
    }
}

void
MRAM::logEvent(MRAM::Event::Address address, MRAM::Event::Data expected, MRAM::Event::Data read1, MRAM::Event::Data read2, MRAM::Event::State state) {
    Event::Data diff = expected ^ read1;
    uint32_t flips = __builtin_popcount(diff);

    Event::GuessedType guessedType = Event::SEFI;

    if (flips == 1) {
        if (read1 == read2) {
            guessedType = Event::SEU;
        } else {
            guessedType = Event::SET;
        }
    } else if (flips < 4) {
        if (read1 == read2) {
            guessedType = Event::MBU;
        } else {
            guessedType = Event::SET;
        }
    } else {
        guessedType = Event::SEFI;
    }

    Event event {
        guessedType,
        state,
        flips,
        address,
        expected,
        read1,
        read2,
        currentDatetimeMilliseconds().str(),
        formatDuration(std::chrono::milliseconds(microcontrollerClock.load())).str(),
        currentExperimentTime().str()
    };

    timeLog.push_back(event);

    beep->beep(Beep::BeepType::Soft);

    csv->addCSVentry("mram", std::vector<std::string>{
            std::string(magic_enum::enum_name(state)),
            std::string(magic_enum::enum_name(guessedType)),
            std::to_string(flips),
            std::to_string(expected),
            std::to_string(read1),
            std::to_string(read2),
    });
}

ImColor MRAM::Event::colour() const {
    switch (guessedType) {
        case SET:
            return ImColor::HSV(0.37f, 0.7f, 0.9f);
        case SEFI:
            return ImColor::HSV(0.62f, 0.7f, 0.9f);
        case SEU:
            return ImColor::HSV(0.15f, 0.8f, 0.87f);
        case MBU:
            return ImColor::HSV(0.09f, 0.8f, 0.87f);
        case SEL:
            return ImColor::HSV(0.96f, 0.9f, 0.9f);
        default:
            return ImColor::HSV(0.0f, 0.0f, 0.5f);
    }
}

