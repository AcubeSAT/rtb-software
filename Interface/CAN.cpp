#include <imgui.h>
#include "CAN.h"
#include "Clock.h"

void CAN::logEvent(CAN::Event::Data rx, CAN::Event::Data tx, CAN::Event::MeasuredType type) {
    const std::lock_guard lock(timeLogMutex);

    Event::Data diff = rx ^ tx;
    int flips = __builtin_popcount(diff);

    Event::GuessedType guessedType = Event::SEFI;
    if (flips == 1) {
        guessedType = Event::SET;
    } else if (flips <= 4) {
        guessedType = Event::MBU;
    } else {
        guessedType = Event::SEFI;
    }

    Event event {
        type,
        CAN::Event::SET,
        static_cast<uint32_t>(flips),
        rx,
        tx,
        currentDatetimeMilliseconds().str(),
        formatDuration(std::chrono::milliseconds(microcontrollerClock.load())).str()
    };

    timeLog.push_back(event);
}

void CAN::window() {
    ImGui::Text("Bytes TX: %05ld\tBytes RX: %05ld", 0, 0);
    ImGui::Text("Packets TX: %05ld\tPackets RX: %05ld", 0, 0);
    ImGui::Spacing();

    ImGui::Text("RX:");
    ImGui::SameLine(0.0f, 10.0f);
    ImGui::Text("0xFADI1E0ACBJFADFI");
    
    ImGui::Text("RX:");
    ImGui::SameLine(0.0f, 10.0f);
    ImGui::Text("0xFADI1E0ACBJFADFI");

    ImGui::Separator();
    ImGui::Text("CAN error log");
    ImGui::Spacing();
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("table_can_timelog", 3, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make header row always visible
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("MCU Time", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("#flips", ImGuiTableColumnFlags_WidthFixed, 100);
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
                ImGui::Text("%s", timeLog[index].mcuTime.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", timeLog[index].flips);
            }
        }
        ImGui::EndTable();
    }
}