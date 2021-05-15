#ifndef RTB_SOFTWARE_CAN_H
#define RTB_SOFTWARE_CAN_H

#include <stdint.h>

void Experiment_CAN_Start();
void Experiment_CAN_Loop();
void Experiment_CAN_Silent_Loop();
void Experiment_CAN_Stop();
void Experiment_CAN_Reset();

void Experiment_CAN_DeinitPins();

#endif //RTB_SOFTWARE_CAN_H
