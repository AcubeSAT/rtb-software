#ifndef RTB_SOFTWARE_SERIALHANDLER_H
#define RTB_SOFTWARE_SERIALHANDLER_H

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <optional>
#include <fstream>
#include "Log.h"
#include "Clock.h"

class SerialHandler {
private:
    boost::asio::streambuf receivedData;
    bool popupOpen = false;

    void receiveHandler(const boost::system::error_code& error, std::size_t size);
    void transmitHandler(const boost::system::error_code& error, std::size_t size) {
        dataSent = true;
        dataSending = false;
    }

    static std::ostringstream time();

    bool dataSending = false;
    bool dataSent = false;
    bool dataReceived = false;
    bool dataError = false;

     std::optional<std::reference_wrapper<Log>> log;
     std::optional<std::ofstream> file;
public:
    explicit SerialHandler(Log &log) : log(log) {
        file.emplace();
        file->open(getLogFileName("device").str(), std::ios::out | std::ios::app | std::ios::binary);
    };

    std::unique_ptr<boost::asio::io_service> io;
    std::unique_ptr<boost::asio::serial_port> serial;
    std::string port;

    /**
     * Sends a message to the serial device
     * Note: This does not append a newline or null byte to the message
     * @todo Handle many messages being written concurrently
     */
    void write(std::string message);

    /**
     * The main thread function. Use std::thread() on this in order to keep the serial handler running in the background
     */
    void thread();

    /**
     * Commands the main ASIO IO worker to stop working
     */
    void stop();

    /**
     * Designs the ImGui window for the serial stuff
     */
     void window();
};


#endif //RTB_SOFTWARE_SERIALHANDLER_H
