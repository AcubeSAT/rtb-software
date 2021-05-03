#ifndef RADIATIONINTERFACE_PARAMETERS_H
#define RADIATIONINTERFACE_PARAMETERS_H

#include <string>
#include <array>
#include <any>
#include <magic_enum.hpp>
#include <utility>
#include "cereal/cereal.hpp"
#include <cereal/archives/json.hpp>


class EnumParameterBase {
public:
    virtual int intValue() const = 0;

    virtual int count() const = 0;

    virtual std::string valueText() const = 0;

    virtual std::string name() const = 0;

    virtual void setValue(int newValue) = 0;

    virtual void reset() = 0;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(cereal::make_nvp("name", name()), cereal::make_nvp("value", intValue()));
    }
};

template<typename T>
class Parameter {
public:
    std::string name;
    T defaultValue;
    T value;

    T min;
    T max;

    std::function<void(T)> callback;

    void callCallback() {
        if (callback) callback(value);
    }

    Parameter(std::string name, T value, const std::function<void(T)> &callback = {}) : name(std::move(name)),
                                                                                        defaultValue(value),
                                                                                        value(value),
                                                                                        callback(callback) {}

    Parameter(std::string name, T value, T min, T max, const std::function<void(T)> &callback = {}) : name(
            std::move(name)),
                                                                                                      defaultValue(
                                                                                                              value),
                                                                                                      value(value),
                                                                                                      min(min),
                                                                                                      max(max),
                                                                                                      callback(
                                                                                                              callback) {}

    virtual ~Parameter() = default;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(name), CEREAL_NVP(value));
    }
};

template<typename Enum>
class EnumParameter : public Parameter<Enum>, public EnumParameterBase {
    using Parameter<Enum>::Parameter;
public:
    int intValue() const override {
        return static_cast<int>(Parameter<Enum>::value);
    }

    int count() const override {
        return magic_enum::enum_count<Enum>();
    }

    std::string valueText() const override {
        return std::string(magic_enum::enum_name(Parameter<Enum>::value));
    }

    std::string name() const override {
        return Parameter<Enum>::name;
    }

    void setValue(int newValue) override {
        Parameter<Enum>::value = static_cast<Enum>(newValue);
    }

    void reset() override {
        Parameter<Enum>::value = Parameter<Enum>::defaultValue;
    }

    template<class Archive>
    void serialize(Archive &ar) {
        ar(cereal::base_class<EnumParameterBase>(this));
    }
};

extern std::array<Parameter<float>, 3> floatingParameters;
extern std::array<Parameter<int>, 1> integerParameters;
extern std::array<std::shared_ptr<EnumParameterBase>, 3> enumParameters;

void parameterWindow();

void resetParameters();

void updateParameters();

#endif //RADIATIONINTERFACE_PARAMETERS_H
