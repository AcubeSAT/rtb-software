#ifndef RADIATIONINTERFACE_LOG_H
#define RADIATIONINTERFACE_LOG_H

#include <string>
#include <boost/circular_buffer.hpp>
#include <mutex>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Util.h>

class Log {
    const int MaxLogEntries = 1000;

    bool scrollToBottom = true;
    ImGuiTextFilter filter;

    boost::circular_buffer<std::string> items;

    std::mutex itemMutex;
public:
    Log() : items(MaxLogEntries) {}

    /**
     * Draw the ImGui window
     */
    void window();

    void addLogEntry(const std::string & entry);

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


#endif //RADIATIONINTERFACE_LOG_H
