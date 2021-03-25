#include <imgui.h>
#include "Log.h"
#include "main.h"
#include "imgui_coloured.h"

void Log::window() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
    filter.Draw(R"(Filter ("incl,-excl") ("error"))", 100);

    if (ImGui::GetContentRegionAvail().x > 500) {
        ImGui::SameLine(0, 100);
    } else {
        ImGui::SameLine();
    }

    ImGui::Checkbox("autoscroll", &scrollToBottom);

    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::PopStyleVar();

    ImGui::Separator();

    ImGui::BeginChild("ScrollingRegion", ImVec2(0,-ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::Selectable("Clear")) {}
        ImGui::EndPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,0.5)); // Tighten spacing
    ImGui::PushFont(logFont);
    {
        const std::lock_guard<std::mutex> lock(itemMutex);

        for (auto &it : items) {
            const char* item = it.c_str();
            if (!filter.PassFilter(item)) {
                continue;
            }
            ImVec4 col = ImVec4(0.5,0.4,0.3,0.5); // wat
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            ImGui::TextAnsiUnformatted(item);
            ImGui::PopStyleColor();
        }
    }

    if (scrollToBottom) {
        ImGui::SetScrollY(ImGui::GetScrollMaxY());
    }
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();
}

void Log::addLogEntry(const std::string & entry) {
    const std::lock_guard<std::mutex> lock(itemMutex);

    items.push_back(entry);
}


void Log::LogAppender::write(const plog::Record &record) {
    plog::util::nstring str = plog::TxtFormatter::format(record);

    log.addLogEntry(getColor(record.getSeverity()) + str);
}

std::string Log::LogAppender::getColor(plog::Severity severity) {
    switch (severity) {
        case plog::fatal:
            return "\x1B[37m\x1B[41m"; // white on red background
            break;

        case plog::error:
            return "\x1B[31m"; // red
            break;

        case plog::warning:
            return "\x1B[33m"; // yellow
            break;

        case plog::info:
        case plog::debug:
        case plog::verbose:
            return "\x1B[36m"; // cyan
            break;
        default:
            return "";
    }
}