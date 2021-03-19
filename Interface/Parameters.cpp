#include <imgui.h>
#include <sstream>
#include <boost/asio/buffer.hpp>
#include <boost/asio.hpp>
#include <plog/Log.h>
#include "Parameters.h"
#include "main.h"

std::array<Parameter<float>, 2> floatingParameters = {
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
        ImGui::SliderFloat(parameter.name.c_str(), &parameter.value, parameter.min, parameter.max);
    }

    ImGui::Separator();
    ImGui::Text("Integer parameters");
    ImGui::Spacing();

    for (auto& parameter : integerParameters) {
        ImGui::SliderInt(parameter.name.c_str(), &parameter.value, parameter.min, parameter.max);
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

    LOG_DEBUG << serialCommand;

    // Send the serial command
    boost::asio::async_write(*serial, boost::asio::buffer(serialCommand.c_str(), serialCommand.size()), txHandler);
}
