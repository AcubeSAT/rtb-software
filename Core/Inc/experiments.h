#ifndef RTB_SOFTWARE_EXPERIMENTS_H
#define RTB_SOFTWARE_EXPERIMENTS_H

#include <stdbool.h>
#include <stdint.h>

extern int currentExperiment;

void start_experiment(int experiment);
void stop_experiment(int experiment);
void reset_experiment(int experiment);
void loop_experiment(int experiment);
bool uart_experiment(char* command, uint16_t len);

#endif //RTB_SOFTWARE_EXPERIMENTS_H
