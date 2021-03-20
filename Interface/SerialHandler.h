#ifndef RTB_SOFTWARE_SERIALHANDLER_H
#define RTB_SOFTWARE_SERIALHANDLER_H

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

class SerialHandler {
private:
    boost::asio::streambuf receivedData;
    bool popupOpen = false;

    void receiveHandler(const boost::system::error_code& error, std::size_t size);
    void transmitHandler(const boost::system::error_code& error, std::size_t size) {}

    std::ostringstream time();
public:
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
};


#endif //RTB_SOFTWARE_SERIALHANDLER_H
