#include <imgui.h>
#include <sstream>
#include <boost/asio/buffer.hpp>
#include <boost/asio.hpp>
#include <plog/Log.h>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <boost/algorithm/string.hpp>
#include <variant>
#include "Parameters.h"
#include "main.h"
#include "CommonEnums.h"
#include "Utilities.h"
#include "FontAwesome.h"

static bool unsavedParameters = true;

std::array<Parameter<float>, 4> floatingParameters = {
        Parameter<float>{"Board Voltage", 3.3, 1, 4, "V",[](float voltage) {
            floatingParameters[1].max = voltage;
            floatingParameters[2].max = voltage;
        }},
        Parameter<float>{"LCL V threshold", 0.3, 0, 3.3, "V"},
        Parameter<float>{"LCL Offset voltage", 0.15, 0, 1, "V"},
        Parameter<float>{"Power-cycle delay", 20, 0, 500, "ms"},
};

std::array<Parameter<int>, 1> integerParameters = {
        Parameter<int>{"nil", 0, 0, 0},
};

std::array<std::shared_ptr<EnumParameterBase>, 3> enumParameters = {
    std::dynamic_pointer_cast<EnumParameterBase>(std::make_shared<EnumParameter<parameters::CANSpeed>>(std::string("CAN baudrate"), parameters::CANSpeed::baud250kbps)),
    std::dynamic_pointer_cast<EnumParameterBase>(std::make_shared<EnumParameter<parameters::Latchupinator>>(std::string("Latchup simulation"), parameters::Latchupinator::RandomErrorsOFF)),
    std::dynamic_pointer_cast<EnumParameterBase>(std::make_shared<EnumParameter<parameters::TakeMeasurements>>(std::string("Measurement sending"), parameters::TakeMeasurements::MeasurementsON))
};

namespace cereal {
    // Serialise a variant without printing the index
    template<class Archive, typename VariantType1, typename... VariantTypes>
    void save(Archive &ar, std::variant<VariantType1, VariantTypes...> const &variant) {
        std::visit([&ar](auto arg) { ar(arg); }, variant);
    }

    template<class Archive, typename... VariantTypes>
    inline
    void load(Archive &ar, std::variant<VariantTypes...> &variant) {}

    // Don't encapsulate variants and maps
    template<typename... VariantTypes>
    void epilogue(cereal::JSONOutputArchive&, const std::variant<VariantTypes...> &){}
    template<typename... VariantTypes>
    void prologue(cereal::JSONOutputArchive&, const std::variant<VariantTypes...> &){}
    template<typename K, typename T>
    void epilogue(cereal::JSONOutputArchive&, const std::map<K,T> &){}
    template<typename K, typename T>
    void prologue(cereal::JSONOutputArchive&, const std::map<K,T> &){}
}

void parameterWindow() {
    ImGui::Text("Set Parameters");
    ImGui::SameLine();
    HelpMarker("Use Ctrl+Click to manually set values");
    ImGui::Spacing();

    for (auto& parameter : floatingParameters) {
        std::string format = parameter.units.empty() ? "%.3f" : "%.3f " + parameter.units;

        if (ImGui::SliderScalar(parameter.name.c_str(), ImGuiDataType_Float, &parameter.value, &parameter.min, &parameter.max, format.c_str(), ImGuiSliderFlags_None)) {
            parameter.callCallback();
            unsavedParameters = true;
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    for (auto& parameter : integerParameters) {
        if (ImGui::SliderInt(parameter.name.c_str(), &parameter.value, parameter.min, parameter.max)) {
            parameter.callCallback();
            unsavedParameters = true;
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    for (auto& parameter : enumParameters) {
        try {
            int formValue = parameter->intValue();

            if (ImGui::SliderInt(parameter->name().c_str(), &formValue, 0, parameter->count() - 1,
                             parameter->valueText().c_str())) {
                parameter->setValue(formValue);
                unsavedParameters = true;
            }
        } catch (const std::exception & e) {
            LOG_FATAL << e.what();
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Reset")) {
        resetParameters();
    }
    HelpTooltip("Bring all parameters to their original values. This will not immediately update the parameters in the MCU.");
    ImGui::SameLine();
    if (ImGui::Button("Update all", ImVec2(-FLT_MIN, 0.0f))) {
        updateParameters();
    }

    if (unsavedParameters) {
        ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.15, 0.6f, 0.8f));
        FontAwesomeText(FontAwesome::ExclamationTriangle);
        ImGui::SameLine();
        ImGui::Text("You have unsaved parameters!");
        ImGui::PopStyleColor();
    }

    if (std::dynamic_pointer_cast<EnumParameter<parameters::Latchupinator>>(enumParameters[1])->value == parameters::Latchupinator::RandomErrorsON) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)ImColor::HSV(0, 1.0f, 0.835f));
        ImGui::BeginChild("ErrorWarning", ImVec2(0, 38), true);
        FontAwesomeText(FontAwesome::ExclamationTriangle);
        ImGui::SameLine();
        ImGui::Text("Random errors are ON. All results are inaccurate!");
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
}

std::string dumpParameters() {
    std::ostringstream ss;

    std::map<std::string, std::variant<int, float, std::string>> parameters;

    for (auto& parameter : floatingParameters) {
        parameters[parameter.name] = parameter.value;
    }
    for (auto& parameter : integerParameters) {
        parameters[parameter.name] = parameter.value;
    }
    for (auto& parameter : enumParameters) {
        parameters[parameter->name()] = parameter->valueText();
    }

    cereal::JSONOutputArchive archive(ss, cereal::JSONOutputArchive::Options::Default());
    archive(parameters);

    return ss.str();
}

void updateParameters() {
    LOG_DEBUG << "Updating parameters...";
    LOG_VERBOSE << "Parameter dump: " << boost::replace_all_copy(dumpParameters(), "\n", " ");

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
    index = 0;
    for (auto& parameter : enumParameters) {
        ss << 'n' << " " << (index++) << " " << parameter->intValue() << '\n';
    }

    std::string serialCommand = ss.str();
    serialHandler->write(serialCommand);

    unsavedParameters = false;
}

void resetParameters() {
    for (auto& parameter : floatingParameters) {
        parameter.value = parameter.defaultValue;
        parameter.callCallback();
    }
    for (auto& parameter : integerParameters) {
        parameter.value = parameter.defaultValue;
        parameter.callCallback();
    }
    for (auto& parameter : enumParameters) {
        parameter->reset();
    }

    unsavedParameters = true;
}