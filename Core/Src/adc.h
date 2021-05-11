#ifndef RTB_SOFTWARE_ADC_H
#define RTB_SOFTWARE_ADC_H

extern uint32_t last_converted_values[100];
extern uint32_t converted_values[100];

void ADC_BeginConversion();

#endif //RTB_SOFTWARE_ADC_H
