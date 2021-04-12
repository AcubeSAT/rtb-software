#ifndef RADIATIONINTERFACE_LOG_H
#define RADIATIONINTERFACE_LOG_H

#include <string>
#include <boost/circular_buffer.hpp>
#include <mutex>
#include <utility>
#include "imgui.h"
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Util.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Converters/NativeEOLConverter.h>

class Log {
public:
    struct LogLevel {
        std::string name;
        int severity;
    };
private:
    const int MaxLogEntries = 1000000;

    bool scrollToBottom = true;
    ImGuiTextFilter filter;

    boost::circular_buffer<std::pair<int, std::string>> items;

    std::mutex itemMutex;

    std::vector<LogLevel> logLevels;

    int minSeverity = std::numeric_limits<int>::min();
public:
    explicit Log(std::vector<LogLevel> logLevels) : items(MaxLogEntries), logLevels(std::move(logLevels)) {
        minSeverity = this->logLevels.front().severity;
    }

    /**
     * Draw the ImGui window
     */
    void window();

    void addLogEntry(const std::string & entry, int severity);

    const std::vector<LogLevel>& getLogLevels() const {
        return logLevels;
    }

    class LogAppender : public plog::ColorConsoleAppender<plog::TxtFormatter> {
    private:
        Log& log;
    public:
        LogAppender(plog::OutputStream outStream) = delete;

        explicit LogAppender(Log& log) : log(log) {}
    protected:
        void write(const plog::Record& record) override;
        std::string getColor(plog::Severity severity);
    };

};

class LogControl {
    static plog::RollingFileAppender<plog::TxtFormatter, plog::NativeEOLConverter<>> logAppender;
    static std::string getLogFileDirectory(bool updateDate = false);
public:
    enum class LogStatus {
        manual,
        automatic
    };

    inline static LogStatus status = LogStatus::automatic;
    inline static bool dryRun = false;
    inline static std::string temporaryLogTitle;
    inline static std::string extra = "";

    static void logTitleWindow();
    static void customEntryWindow();

    static void saveNewLogTitle();
    static void reset();
    static std::string getAutomaticLogTitle();

    static void createLogDirectory();
    static std::string getLogFileName(const std::string& type, const std::string& extension = "log");
};


#endif //RADIATIONINTERFACE_LOG_H
