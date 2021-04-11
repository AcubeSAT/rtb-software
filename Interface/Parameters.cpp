#include <imgui.h>
#include <sstream>
#include <boost/asio/buffer.hpp>
#include <boost/asio.hpp>
#include <plog/Log.h>
#include "Parameters.h"
#include "main.h"
#include "CommonEnums.h"

std::array<Parameter<float>, 3> floatingParameters = {
        Parameter<float>{"Board Voltage", 3.3, 1, 4, [](float voltage) {
            floatingParameters[1].max = voltage;
            floatingParameters[2].max = voltage;
        }},
        Parameter<float>{"DAC 1", 0, 0, 3.3},
        Parameter<float>{"DAC 2", 0, 0, 3.3},
};

std::array<Parameter<int>, 1> integerParameters = {
        Parameter<int>{"DAC 1", 0, 0, 0},
};

std::array<std::shared_ptr<EnumParameterBase>, 1> enumParameters = {
    std::dynamic_pointer_cast<EnumParameterBase>(std::make_shared<EnumParameter<parameters::CANSpeed>>(std::string("CAN baudrate"), parameters::CANSpeed::baud250kbps))
};

void parameterWindow() {
    ImGui::Text("Set Parameters");
    ImGui::SameLine();
    HelpMarker("Use Ctrl+Click to manually set values");
    ImGui::Spacing();

    for (auto& parameter : floatingParameters) {
//        ImGui::SliderScalar(parameter.name.c_str(), ImGuiDataType_Double, &parameter.value, &parameter.min, &parameter.max, "%f");
        if (ImGui::SliderFloat(parameter.name.c_str(), &parameter.value, parameter.min, parameter.max)) {
            parameter.callCallback();
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    for (auto& parameter : integerParameters) {
        if (ImGui::SliderInt(parameter.name.c_str(), &parameter.value, parameter.min, parameter.max)) {
            parameter.callCallback();
        }
    }

    ImGui::Separator();
    ImGui::Spacing();
//
    for (auto& parameter : enumParameters) {
        try {
            int formValue = parameter->intValue();

            if (ImGui::SliderInt(parameter->name().c_str(), &formValue, 0, parameter->count() - 1,
                             parameter->valueText().c_str())) {
                parameter->setValue(formValue);
            }
        } catch (const std::exception & e) {
            LOG_FATAL << e.what();
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Update all", ImVec2(-FLT_MIN, 0.0f))) {
        updateParameters();
    };
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
    for (auto& parameter : enumParameters) {
        ss << 'd' << " " << (index++) << " " << parameter->intValue() << '\n';
    }

    std::string serialCommand = ss.str();
    serialHandler->write(serialCommand);
}
