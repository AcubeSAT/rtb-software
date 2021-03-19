#ifndef RADIATIONINTERFACE_MAIN_H
#define RADIATIONINTERFACE_MAIN_H

// Note: This is not a great idea, and it would be better if a wrapper function and queue manager was used instead.
// However, it will do the trick for now, as long as no 2 TXs are run at the same time.
extern std::unique_ptr<boost::asio::serial_port> serial;

inline void txHandler(const boost::system::error_code& error, std::size_t size) {}

#endif //RADIATIONINTERFACE_MAIN_H
