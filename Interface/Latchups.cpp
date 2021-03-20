#include <imgui.h>
#include <plog/Log.h>
#include "Latchups.h"
#include "main.h"

void Latchups::window() {
    float counterHeight = std::max<float>(100, ImGui::GetContentRegionAvail().y * 0.45f);

    ImGui::BeginGroup();
    float currentPosition = ImGui::GetCursorPosY();
    ImGui::SetCursorPosY(currentPosition + (counterHeight - ImGui::CalcTextSize("SEL counter:").y) / 2.0f);
    ImGui::Text("SEL counter:");

    ImGui::PushFont(largeFont);
    ImGui::SameLine();
    ImGui::SetCursorPosY(currentPosition);
    ImGui::Button(std::to_string(latchupCounter).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, counterHeight));
    ImGui::PopFont();
    ImGui::EndGroup();
}
