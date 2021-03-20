#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <imgui.h>
#include "Clock.h"
#include "main.h"

std::stringstream currentDatetime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss;
}

std::stringstream currentDatetimeMilliseconds()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto in_time_t = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1s;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss;
}

template<typename T>
std::stringstream formatDuration(std::chrono::duration<T> ns)
{
    std::stringstream ss;

    using namespace std;
    using namespace std::chrono;
    typedef duration<int, ratio<86400>> days;
    char fill = ss.fill();
    ss.fill('0');
    auto h = duration_cast<hours>(ns);
    ns -= h;
    auto m = duration_cast<minutes>(ns);
    ns -= m;
    auto s = duration_cast<milliseconds>(ns);
    ss << setw(2) << h.count() << ":"
       << setw(2) << m.count() << ":"
       << setw(4) << fixed << setprecision(1) << (s.count() / 1000.0f);
    ss.fill(fill);
    return ss;
};

template std::stringstream formatDuration<double>(std::chrono::duration<double> ns);

void clockWindow() {
    std::string text = currentDatetime().str();

    ImGui::PushFont(largeFont);
    // Place text to the middle
    ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - (ImGui::CalcTextSize(text.c_str()).x)
            - ImGui::GetScrollX()) / 2.0f
            );
    ImGui::Text("%s", text.c_str());
    ImGui::PopFont();
}
