#ifndef RADIATIONINTERFACE_LOG_H
#define RADIATIONINTERFACE_LOG_H

#include <string>
#include <boost/circular_buffer.hpp>
#include <mutex>
#include <utility>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Util.h>

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

    static void customEntryWindow();
};


#endif //RADIATIONINTERFACE_LOG_H
