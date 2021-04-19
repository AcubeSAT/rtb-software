#include <imgui.h>
#include <plog/Log.h>
#include <imgui_internal.h>
#include "Log.h"
#include "main.h"
#include "imgui_coloured.h"
#include "imgui_stdlib.h"
#include "FontAwesome.h"
#include "Experiment.h"
#include <filesystem>
#include "plog/Init.h"
#include "Utilities.h"
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

void Log::window() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
    filter.Draw("filter", 100);

    if (ImGui::GetContentRegionAvail().x > 800) {
        ImGui::SameLine(0, 40);
    } else {
        ImGui::SameLine();
    }

    if (FontAwesomeButton(FontAwesome::Trash)) {
        ImGui::OpenPopup((std::string("Reset Log ") + name).c_str());
    }
    HelpTooltip("Clear all log outputs");

    if (PopupModal(std::string("Reset Log ") + name, "Do you want to clear all log entries?\nLog outputs will still remain on the files.")) {
        this->reset();
    }

    ImGui::SameLine();
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

void Log::reset() {
    items.clear();
}

void Log::LogAppender::write(const plog::Record &record) {
    tm t;
    plog::util::localtime_s(&t, &record.getTime().time);
    plog::util::nostringstream ss;
    ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
    ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << static_cast<int> (record.getTime().millitm) << PLOG_NSTR(" ");
    ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
    ss << record.getMessage() << PLOG_NSTR("\n");

    log.addLogEntry(getColor(record.getSeverity()) + ss.str(), - static_cast<int>(record.getSeverity()));
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

void LogControl::customEntryWindow() {
    static char entry[256] = "";

    ImGuiIO& imguiIo = ImGui::GetIO();

    bool pressed = ImGui::InputText("", entry, 256, ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();
    if (ImGui::Button("Log", ImVec2(-1.0f, 0.0f)) || pressed) {
        LOG_WARNING << entry;
        settings->logShortcuts.back() = entry; // Last button is always the last entry
        strcpy(entry, "");
        settings->flush();
    };

    for (auto& it: settings->logShortcuts) {
        bool button = it.empty() ? ImGui::Button(" ") : ImGui::Button(it.c_str());
        if (button) {
            if (imguiIo.KeyCtrl) {
                it = entry;
                settings->flush();
            } else {
                LOG_WARNING << it;
            }
        }
        ImGui::SameLine();
    }
    HelpMarker("Press Ctrl+Click to set button content");
}

void LogControl::logTitleWindow() {
    bool checkbox = status == LogStatus::manual;
    if (ImGui::Checkbox("manual", &checkbox)) {
        status = checkbox ? LogStatus::manual : LogStatus::automatic;
        reset();
    }
    ImGui::SameLine();

    if (status == LogStatus::manual) {
        ImGui::SameLine(0, 40);
        ImGui::Text("Log Title:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("", &temporaryLogTitle);
    } else {
        ImGui::SameLine();
        if (ImGui::Checkbox("dry run?", &dryRun)) {
            temporaryLogTitle = getAutomaticLogTitle();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50);
        if (ImGui::InputTextWithHint("", "extra", &extra)) {
            temporaryLogTitle = getAutomaticLogTitle();
        }
    }

    if (temporaryLogTitle != settings->logTitle) {
        if (status == LogStatus::manual) {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV(2.5f / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV(2.5f / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV(2.5f / 7.0f, 0.8f, 0.8f));

            ImGui::SameLine();
            ImGui::PushFont(iconFont);

            if (ImGui::Button(FontAwesome::Undo)) {
                reset();
            }

            ImGui::PopFont();
            HelpTooltip("Reset to current title");
            ImGui::PopStyleColor(3);
        }
    }

    if (status == LogStatus::manual || temporaryLogTitle != settings->logTitle) {
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.5f / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.5f / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.5f / 7.0f, 0.8f, 0.8f));

        ImGui::SameLine();
        ImGui::PushFont(iconFont);
        if (ImGui::Button(FontAwesome::Save)) {
            saveNewLogTitle();
        }
        ImGui::PopFont();
        HelpTooltip("Save new log title");

        ImGui::PopStyleColor(3);
    }

    if (status == LogStatus::automatic) {
        ImGui::PushFont(logFont);
        ImGui::TextUnformatted(getAutomaticLogTitle().c_str());
        ImGui::PopFont();
    }
}

std::string LogControl::getLogFileDirectory(bool updateDate) {
    static std::string cached;
    if (cached.empty() || updateDate) {
        stringstream ss;
        ss << currentDatetime("%FT%T").rdbuf() << "." << settings->logTitle;
        cached = ss.str();
    }

    return cached;
}

std::string LogControl::getLogFileName(const string& type, const string& extension) {
    stringstream ss;

    ss << "log/" << getLogFileDirectory();

    if (!type.empty()) {
        ss << "/" << settings->logTitle << "." << type;
    }

    ss << "." << extension;

    return ss.str();
}

void LogControl::createLogDirectory() {
    try {
        std::filesystem::create_directory("log");
        std::filesystem::create_directory("log/" + getLogFileDirectory());
    } catch (const std::exception& e) {
        LOG_FATAL << "Could not create log directory: " << e.what();
    }
}


void LogControl::reset() {
    if (status == LogStatus::manual) {
        temporaryLogTitle = settings->logTitle;
    } else {
        temporaryLogTitle = getAutomaticLogTitle();
    }
}

std::string LogControl::getAutomaticLogTitle() {
    Experiment experiment = Experiment::getCurrentExperiment();
    return experiment.name + (dryRun ? ".Dry" : "") + (extra.empty() ? "" : ("." + extra));
}

void LogControl::saveNewLogTitle() {
    settings->logTitle = temporaryLogTitle;
    settings->flush();
    getLogFileDirectory(true);

    LOG_WARNING << "Moving to new log file " << getLogFileName("***");

    createLogDirectory();

    logFileAppender->setFileName(LogControl::getLogFileName("host").c_str());
    serialHandler->openLogFile();
    csv->refreshAllFilenames();
}
