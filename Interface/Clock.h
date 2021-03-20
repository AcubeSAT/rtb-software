#ifndef RADIATIONINTERFACE_CLOCK_H
#define RADIATIONINTERFACE_CLOCK_H

#include <sstream>

std::stringstream currentDatetime();
std::stringstream currentDatetimeMilliseconds();
template<typename T> std::stringstream formatDuration(std::chrono::duration<T> ns);

void clockWindow();



#endif //RADIATIONINTERFACE_CLOCK_H
