#ifndef RADIATIONINTERFACE_CSV_H
#define RADIATIONINTERFACE_CSV_H

#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <any>
#include <thread>
#include <boost/lockfree/spsc_queue.hpp>
#include <mutex>
#include <condition_variable>

class CSV {
private:
    inline static const auto flushInterval = std::chrono::milliseconds(2000);

    typedef std::chrono::steady_clock::time_point Timepoint;

    // Map of filename -> (file, last flush time)
    std::map<std::string, std::pair<std::ofstream, Timepoint>> files;
    std::map<std::string, std::vector<std::string>> fileSignatures;

    void createFile(const std::string & filename, bool force = false);
    void createFile(const std::string & filename, const std::vector<std::string> & columns, bool force = false);

    boost::lockfree::spsc_queue<std::pair<std::string,std::string>, boost::lockfree::capacity<128>> queue;
    std::condition_variable queueNotification;
    std::mutex queueMutex;
    std::mutex fileMutex;
    std::atomic_bool stop = false;

    std::thread fileWriterThread;
    void thread();
public:
    CSV();

    void addCSVentry(const std::string& filename, const std::vector<std::string> &data);

    void refreshAllFilenames();
};


#endif //RADIATIONINTERFACE_CSV_H
