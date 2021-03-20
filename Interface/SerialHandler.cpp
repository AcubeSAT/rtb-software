#include <boost/system/error_code.hpp>
#include <plog/Log.h>
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

using namespace std::chrono_literals;
using namespace plog;
using namespace std::chrono;

void SerialHandler::receiveHandler(const boost::system::error_code &error, std::size_t size) {
    if (!error) {
        LOG_VERBOSE << "Read " << size << " bytes of data";

        std::string receivedAll(reinterpret_cast<const char *>(receivedData.data().data()), size);

        // Find the first ocurrence of a newline
        size_t zeroLocation = receivedAll.find('\n');
        std::string receivedRaw(reinterpret_cast<const char *>(receivedData.data().data()), zeroLocation);
        receivedData.consume(zeroLocation + 1);
        // async_read will automatically deals with over-abundance of buffer data, so no need to handle anything after
        // the newline

        std::cout << time().str() << receivedRaw << std::endl;
    } else {
        LOG_ERROR << error;
    }

    boost::asio::async_read_until(*serial, receivedData, '\n', [this](const boost::system::error_code& error, std::size_t size) { receiveHandler(error, size); });
}

void SerialHandler::thread() {
    std::this_thread::sleep_for(500ms);
    LOG_INFO << "Starting data acquisition thread";
//
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
//        if (!popupOpen && ImguiStarted) {
//            popupOpen = true;
//            ImGui::OpenPopup("Connection");
//        }
    } catch (...) {
        std::string exception = typeid(std::current_exception()).name();
        LOG_FATAL << "Unhandled exception in data acquisition thread: " << exception;
    }
}

void SerialHandler::stop() {
    io->stop();
}

void SerialHandler::write(std::string message) {
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
