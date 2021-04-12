#ifndef RADIATIONINTERFACE_CSV_H
#define RADIATIONINTERFACE_CSV_H

#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <any>

class CSV {
private:
    typedef std::chrono::steady_clock::time_point Timepoint;

    std::map<std::string, std::pair<std::ofstream, Timepoint>> files;

    void createFile(const std::string & filename, const std::vector<std::string> & columns);
public:
    CSV();

    void addCSVentry(const std::string& filename, const std::vector<std::any> & data) {
        for (auto it: data) {

        }
    }
};


#endif //RADIATIONINTERFACE_CSV_H
