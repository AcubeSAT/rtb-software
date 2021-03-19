#ifndef RADIATIONINTERFACE_PARAMETERS_H
#define RADIATIONINTERFACE_PARAMETERS_H

#include <string>
#include <array>

template <typename T>
struct Parameter {
    std::string name;
    T value;

    T min;
    T max;
};

extern std::array<Parameter<float>, 2> floatingParameters;

extern std::array<Parameter<int>, 2> integerParameters;

void parameterWindow();

void updateParameters();

#endif //RADIATIONINTERFACE_PARAMETERS_H
