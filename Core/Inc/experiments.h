#ifndef RTB_SOFTWARE_EXPERIMENTS_H
#define RTB_SOFTWARE_EXPERIMENTS_H

#include <stdbool.h>
#include <stdint.h>

extern int currentExperiment;

bool uart_experiment(char* command, uint16_t len);

#endif //RTB_SOFTWARE_EXPERIMENTS_H
