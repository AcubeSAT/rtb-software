#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <imgui.h>
#include "Clock.h"
#include "main.h"

std::string currentDatetime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

void clockWindow() {
    std::string text = currentDatetime();

    ImGui::PushFont(largeFont);
    // Place text to the middle
    ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - (ImGui::CalcTextSize(text.c_str()).x)
            - ImGui::GetScrollX()) / 2.0f
            );
    ImGui::Text("%s", text.c_str());
    ImGui::PopFont();
}
