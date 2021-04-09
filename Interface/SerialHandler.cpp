#include <boost/system/error_code.hpp>
#include <plog/Log.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/system/system_error.hpp>
#include <thread>
#include <boost/asio.hpp>
#include <imgui.h>
#include "SerialHandler.h"
#include <plog/Util.h>
#include <boost/bind.hpp>
#include <iostream>
#include <cstdio>
#include <iomanip>
#include <chrono>
#include <regex>
#include "main.h"
#include "Clock.h"

using namespace std::chrono_literals;
using namespace plog;
using namespace std::chrono;

void SerialHandler::receiveHandler(const boost::system::error_code &error, std::size_t size) {
    if (!error) {
        dataReceived = true;

        std::string receivedAll(reinterpret_cast<const char *>(receivedData.data().data()), size);

        // Find the first ocurrence of a newline
        size_t zeroLocation = receivedAll.find('\n');
        std::string receivedRaw(reinterpret_cast<const char *>(receivedData.data().data()), zeroLocation);
        receivedData.consume(zeroLocation + 1);
        // async_read will automatically deals with over-abundance of buffer data, so no need to handle anything after
        // the newline

        if (!receivedRaw.empty()) {
            if (receivedRaw[0] == UART_CONTROL) {
                if (receivedRaw.size() <= 1) {
                    LOG_WARNING << "Received empty command?";
                    return;
                }

                try {
                    if (receivedRaw[1] == 'l') {
                        latchups.logLatchup();
                    } else if (receivedRaw[1] == 't') {
                        microcontrollerClock = std::stoi(receivedRaw.substr(2));
                    } else if (receivedRaw[1] == 'm') {
                        measurements.acquire(0, std::stof(receivedRaw.substr(2)));
                    } else if (receivedRaw[1] == 'c' ) {
                        // CAN bus error
                        std::stringstream ss(receivedRaw.substr(2));
                        CAN::Event::Data tx;
                        CAN::Event::Data rx;
                        ss >> std::hex >> tx >> rx;

                        can.logEvent(rx, tx);
                    } else if (receivedRaw[1] == 's') {
                        // Statistics
                        std::stringstream ss(receivedRaw.substr(2));

                        std::string type;
                        ss >> type;

                        if (type == "CAN") {
                            CAN::Stats stats{};

                            ss >> stats.txBytes >> stats.txPackets >> stats.rxBytes >> stats.rxPackets;
                            can.setStats(stats);
                        }
                    } else {
                        LOG_WARNING << "Unknown command " << receivedRaw[1] << " received";
                    }
                } catch (std::exception &e) {
                    LOG_ERROR << "Error decoding message: " << e.what();
                }
            } else {
                if (log.has_value()) {
                    std::istringstream ss;
                    std::vector<std::string> words;

                    std::regex ansiControls("\033\\[(.*?)m");
                    std::string receivedWithoutAnsi = std::regex_replace(receivedRaw, ansiControls, "");
                    boost::algorithm::trim(receivedWithoutAnsi);

                    boost::algorithm::split(words, receivedWithoutAnsi, boost::is_any_of("\t "));
                    int severity = -1;

                    if (words.size() > 1) {
                        std::string entryLogLevel = words[1];
                        // Search for the severity in one of the predefined log levels
                        for (auto &it : log->get().getLogLevels()) {
                            if (entryLogLevel == it.name) { // Takes care of ANSI codes
                                severity = it.severity;
                                break;
                            }
                        }
                    }

                    if (severity < 0) {
                        LOG_DEBUG << "Could not parse device log message";
                        severity = 4;
                    }

                    if (severity != 0) { // TODO: Adaptive based on selection?
                        std::cout << time().str() << receivedRaw << std::endl;
                    }

                    log.value().get().addLogEntry(time().str() + receivedRaw, severity);
                }

                if (file.has_value()) {
                    file.value() << time().str() << receivedRaw << std::endl;
                    file->flush();
                }
            }
        }
    } else {
        LOG_ERROR << error;
        dataError = true;
        std::this_thread::sleep_for(500ms);
    }

    boost::asio::async_read_until(*serial, receivedData, '\n', [this](const boost::system::error_code& error, std::size_t size) { receiveHandler(error, size); });
}

void SerialHandler::thread() {
    std::this_thread::sleep_for(500ms);
    LOG_INFO << "Starting data acquisition thread";

    try {
        // Serial interface initialisation
        io = std::make_unique<boost::asio::io_service>();
        serial = std::make_unique<boost::asio::serial_port>(*io, port);
        serial->set_option(boost::asio::serial_port_base::baud_rate(1000000));

        boost::asio::streambuf buf;

        LOG_INFO << "Serial connection successful";

        boost::asio::async_read_until(*serial, receivedData, '\n', [this](const boost::system::error_code& error, std::size_t size) { receiveHandler(error, size); });
        io->run();
    } catch (boost::system::system_error &e) {
        LOG_FATAL << "Unable to open interface " << port << ": " << e.what();
        dataError = true;
    } catch (std::exception &e) {
        LOG_FATAL << "Unhandled exception in data acquisition thread: " << e.what();
        dataError = true;
    } catch (...) {
        std::string exception = typeid(std::current_exception()).name();
        LOG_FATAL << "Unhandled exception in data acquisition thread: " << exception;
        dataError = true;
    }
}

void SerialHandler::stop() {
    io->stop();
}

void SerialHandler::write(std::string message) {
    if (!serial || !serial->is_open()) {
        LOG_ERROR << "There is an error with the serial port. Cannot write data.";
        return;
    }

    dataSending = true;
    boost::asio::async_write(*serial, boost::asio::buffer(message, message.size()), [this](const boost::system::error_code& error, std::size_t size) { transmitHandler(error, size); });
}

std::ostringstream SerialHandler::time() {
    const bool useUtcTime = false;

    milliseconds ms = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()
    ) % 1s;

    tm t;
    time_t now;
    ::time ( &now );
    useUtcTime ? util::gmtime_s(&t, &now) : util::localtime_s(&t, &now);

    std::ostringstream ss;
    ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
    ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << ms.count() << PLOG_NSTR(" ");
    ss << std::setfill(PLOG_NSTR(' '));

    return ss;
}

void SerialHandler::window() {
    if (ImGui::Button("!!")) {
        write("what");
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.1f, 0.9f, 0.05f, 1.0f}));
    ImGui::RadioButton("RX", dataReceived);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.1f, 0.9f, 0.05f, 1.0f}));
    ImGui::RadioButton("", dataSent);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.9f, 0.3f, 0.05f, 1.0f}));
    ImGui::RadioButton("TX", dataSending);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.9f, 0.4f, 0.05f, 1.0f}));
    ImGui::RadioButton("Error", dataError);
    ImGui::PopStyleColor();


    // Reset indicators so that they light up just for one frame
    dataReceived = dataSent = false;
    dataError = false;
}
