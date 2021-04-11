#include <imgui.h>
#include <plog/Log.h>
#include "Log.h"
#include "main.h"
#include "imgui_coloured.h"

void Log::window() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
    filter.Draw("filter", 100);

    if (ImGui::GetContentRegionAvail().x > 800) {
        ImGui::SameLine(0, 40);
    } else {
        ImGui::SameLine();
    }

    ImGui::Checkbox("autoscroll", &scrollToBottom);

    if (ImGui::GetContentRegionAvail().x > 800) {
        ImGui::SameLine(0, 40);
        ImGui::Text("Log Level:");
    }

    for (auto &it : logLevels) {
        ImGui::SameLine();
        ImGui::RadioButton(it.name.c_str(), &minSeverity, it.severity);
    }

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
            if (it.first < minSeverity) {
                continue;
            }
            const char* item = it.second.c_str();
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

void Log::addLogEntry(const std::string & entry, int severity) {
    const std::lock_guard<std::mutex> lock(itemMutex);

    items.push_back(std::make_pair(severity, entry));
}

void Log::customEntryWindow() {
    static char entry[256] = "";
    static std::array<std::string, 6> buttons {"A", "B", "C", "D", "E", "F"};

    ImGuiIO& imguiIo = ImGui::GetIO();

    bool pressed = ImGui::InputText("", entry, 256, ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();
    if (ImGui::Button("Log", ImVec2(-1.0f, 0.0f)) || pressed) {
        LOG_WARNING << entry;
        buttons.back() = entry; // Last button is always the last entry
        strcpy(entry, "");
    };

    for (auto& it: buttons) {
        if (ImGui::Button(it.c_str())) {
            if (imguiIo.KeyCtrl) {
                if (std::string("") == entry) {
                    it = " ";
                } else {
                    it = entry;
                }
            } else {
                LOG_WARNING << it;
            }
        }
        ImGui::SameLine();
    }
    HelpMarker("Press Ctrl+Click to set button content");
}


void Log::LogAppender::write(const plog::Record &record) {
    plog::util::nstring str = plog::TxtFormatter::format(record);

    log.addLogEntry(getColor(record.getSeverity()) + str, - static_cast<int>(record.getSeverity()));
}

std::string Log::LogAppender::getColor(plog::Severity severity) {
    switch (severity) {
        case plog::fatal:
            return "\x1B[31m"; // white on red background
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