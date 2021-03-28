#include <imgui.h>
#include <sstream>
#include <boost/asio/buffer.hpp>
#include <boost/asio.hpp>
#include <plog/Log.h>
#include "Parameters.h"
#include "main.h"

std::array<Parameter<float>, 3> floatingParameters = {
        Parameter<float>{"Board Voltage", 3.3, 1, 4, [](float voltage) {
            floatingParameters[1].max = voltage;
            floatingParameters[2].max = voltage;
        }},
        Parameter<float>{"DAC 1", 0, 0, 3.3},
        Parameter<float>{"DAC 2", 0, 0, 3.3},
};

std::array<Parameter<int>, 2> integerParameters = {
        Parameter<int>{"n/a 1", 0, 0, 255},
        Parameter<int>{"n/a 2", 0, 0, 255},
};

void parameterWindow() {
    ImGui::Text("Floating Point parameters");
    ImGui::Spacing();

    for (auto& parameter : floatingParameters) {
//        ImGui::SliderScalar(parameter.name.c_str(), ImGuiDataType_Double, &parameter.value, &parameter.min, &parameter.max, "%f");
        if (ImGui::SliderFloat(parameter.name.c_str(), &parameter.value, parameter.min, parameter.max)) {
            parameter.callCallback();
        }
    }

    ImGui::Separator();
    ImGui::Text("Integer parameters");
    ImGui::Spacing();

    for (auto& parameter : integerParameters) {
        if (ImGui::SliderInt(parameter.name.c_str(), &parameter.value, parameter.min, parameter.max)) {
            parameter.callCallback();
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Update all", ImVec2(-FLT_MIN, 0.0f))) {
        updateParameters();
    };

    ImGui::Spacing();
    ImGui::Text("Hint: Ctrl+Click to input value!");
}

void updateParameters() {
    LOG_DEBUG << "Updating parameters...";

    std::ostringstream ss;

    // Prepare the serial command
    int index = 0;
    for (auto& parameter : floatingParameters) {
        ss << 'f' << " " << (index++) << " " << parameter.value << '\n';
    }
    index = 0;
    for (auto& parameter : integerParameters) {
        ss << 'd' << " " << (index++) << " " << parameter.value << '\n';
    }

    std::string serialCommand = ss.str();
    serialHandler->write(serialCommand);
}
