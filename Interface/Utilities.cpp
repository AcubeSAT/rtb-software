#include "Utilities.h"
#include "imgui.h"
#include "FontAwesome.h"
#include "main.h"
#include <string>

void HelpMarker(const std::string &text)  {
    ImGui::TextDisabled("(?)");
    HelpTooltip(text);
}

void HelpTooltip(const std::string &text)  {
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(text.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool FontAwesomeButton(const char *text) {
    ImGui::PushFont(iconFont);
    bool result = ImGui::Button(text);
    ImGui::PopFont();

    return result;
}

void FontAwesomeText(const char *text) {
    ImGui::PushFont(iconFont);
    ImGui::TextUnformatted(text);
    ImGui::PopFont();
}

bool PopupModal(const std::string & name, const std::string & prompt) {
    bool output = false;

    if (ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted(prompt.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            output = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    return output;
}