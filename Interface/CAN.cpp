#include <imgui.h>
#include "CAN.h"
#include "Clock.h"
#include "main.h"
#include <bitset>
#include <plog/Log.h>
#include "magic_enum.hpp"
#include "Utilities.h"
#include "Parameters.h"
#include "Experiment.h"

void CAN::logEvent(CAN::Event::Data rx, CAN::Event::Data tx, CAN::Event::State state, CAN::Event::MeasuredType type, const std::string& info) {
    const std::lock_guard lock(timeLogMutex);

    Event::Data diff = rx ^ tx;
    int flips = __builtin_popcount(diff);

    Event::GuessedType guessedType = Event::SEFI;
    if (type == Event::Timeout) {
        guessedType = Event::SEFI;
    } else if (flips == 1) {
        guessedType = Event::SET;
    } else if (flips < 4) {
        guessedType = Event::MBU;
    } else {
        guessedType = Event::SEFI;
    }

    if (guessedType == Event::SEFI) {
        consecutiveSEFIs++;

        if (integerParameters[0].value != 0 && consecutiveSEFIs >= integerParameters[0].value) {
            consecutiveSEFIs = 0;
            Experiment::sendExperimentCommand(Experiment::ExperimentCommand::PowerCycle);
            guessedType = Event::Hard_SEFI;
        }
    } else {
        consecutiveSEFIs = 0;
    }

    Event event {
        type,
        guessedType,
        state,
        static_cast<uint32_t>(flips),
        rx,
        tx,
        info,
        currentDatetimeMilliseconds().str(),
        formatDuration(std::chrono::milliseconds(microcontrollerClock.load())).str(),
        currentExperimentTime().str()
    };

    timeLog.push_back(event);

    beep->beep(Beep::BeepType::Soft);

    csv->addCSVentry("can", std::vector<std::string>{
            std::string(magic_enum::enum_name(state)),
            std::string(magic_enum::enum_name(type)),
            std::string(magic_enum::enum_name(guessedType)),
            std::to_string(flips),
            std::to_string(rx),
            std::to_string(tx),
            info
    });
}

void CAN::window() {
    Stats loadedStats = this->stats.load();

    ImGui::Text("Bytes TX: %08ld", loadedStats.txBytes);
    ImGui::SameLine(200.0f);
    ImGui::Text("Packets TX: %08ld", loadedStats.txPackets);
    ImGui::Text("Bytes RX: %08ld", loadedStats.rxBytes);
    ImGui::SameLine(200.0f);
    ImGui::Text("Packets RX: %08ld", loadedStats.rxPackets);
    ImGui::Separator();
    ImGui::Spacing();

    {
        const std::lock_guard lock(timeLogMutex);

        if (!timeLog.empty()) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            auto lastItem = timeLog.cend() - 1;

            ImGui::Text("TX:");
            ImGui::SameLine(50.0f, 0.0f);
            ImGui::PushFont(logFont);
            ImGui::Text("%#018lx", lastItem->tx);
            ImGui::PopFont();
            draw_list->AddRectFilled(padMin(ImGui::GetItemRectMin()), padMax(ImGui::GetItemRectMax()), IM_COL32(75, 75, 75, 120));

            ImGui::SameLine(200.0f, 0.0f);
            ImGui::PushFont(logFont);
            ImGui::Text("%s", lastItem->toBits(lastItem->rx).c_str());
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
            ImGui::SetCursorPosY(currentPosition);

            ImGui::Spacing();

            ImGui::Text("RX:");
            ImGui::SameLine(50.0f, 0.0f);
            ImGui::PushFont(logFont);
            ImGui::Text("%#018lx", lastItem->rx);
            ImGui::PopFont();
            draw_list->AddRectFilled(padMin(ImGui::GetItemRectMin()), padMax(ImGui::GetItemRectMax()), IM_COL32(75, 75, 75, 120));

            ImGui::SameLine(200.0f, 0.0f);
            ImGui::PushFont(logFont);
            ImGui::Text("%s", lastItem->toBits(lastItem->tx).c_str());
            ImGui::PopFont();
            draw_list->AddRectFilled(padMin(ImGui::GetItemRectMin()), padMax(ImGui::GetItemRectMax()), IM_COL32(75, 75, 75, 120));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("CAN error log");
    ImGui::Spacing();
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("table_can_timelog", 6, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make header row always visible
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30);
        ImGui::TableSetupColumn("Experiment Time", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("Dir.", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 70);
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
                ImGui::TextColored(timeLog[index].colour(), "%s", guessedType.c_str());

                if (timeLog[index].measuredType == Event::BitFlip) {
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%d", timeLog[index].flips);

                    if (!timeLog[index].info.empty()) {
                        ImGui::TableSetColumnIndex(5);
                        ImGui::Text("%s", timeLog[index].info.c_str());
                    }
                } else {
                    ImGui::TableSetColumnIndex(4);
                    std::string measuredType(magic_enum::enum_name(timeLog[index].measuredType));
                    ImGui::Text("%s", measuredType.c_str());

                    ImGui::TableSetColumnIndex(5);
                    ImGui::Text("%s", timeLog[index].info.c_str());
                }
            }
        }
        ImGui::EndTable();
    }
}

void CAN::reset() {
    const std::lock_guard lock(timeLogMutex);

    LOG_INFO << "CAN Statistics:" << ArchiveDump(stats.load(), "CAN");

    stats.store(Stats());
    timeLog.clear();
}

std::string CAN::Event::toBits(CAN::Event::Data number) const {
    int byteLocation = __builtin_ctz(rx ^ tx) / 8;
    if (byteLocation >= sizeof(number) * 8) {
        byteLocation = 0;
    }
    uint8_t byte = (reinterpret_cast<uint8_t*>(&number))[byteLocation];

    std::bitset<8> bitset(byte);

    std::stringstream ss;
    ss << bitset;

    return ss.str();
}

ImColor CAN::Event::colour() const {
    switch (guessedType) {
        case SET:
            return ImColor::HSV(0.37f, 0.7f, 0.9f);
        case SEFI:
            return ImColor::HSV(0.62f, 0.7f, 0.9f);
        case Hard_SEFI:
            return ImColor::HSV(0.62f, 0.5f, 1.0f);
        case MBU:
            return ImColor::HSV(0.09f, 0.8f, 0.87f);
        case SEL:
            return ImColor::HSV(0.96f, 0.9f, 0.9f);
    }
}
