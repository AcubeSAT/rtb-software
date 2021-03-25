#ifndef RADIATIONINTERFACE_IMGUI_COLOURED_H
#define RADIATIONINTERFACE_IMGUI_COLOURED_H

#include <imgui.h>

namespace ImGui {

    void TextAnsiColored(const ImVec4 &col, const char *fmt, ...);

    void TextAnsiColoredV(const ImVec4 &col, const char *fmt, va_list args);

    void TextAnsiV(const char *fmt, va_list args);

    void TextAnsiUnformatted(const char *text, const char *text_end = nullptr);

}

#endif //RADIATIONINTERFACE_IMGUI_COLOURED_H
