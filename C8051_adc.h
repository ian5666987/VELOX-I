// code by ntu ael

#ifndef C8051_ADC_H
#define C8051_ADC_H

void ADC0_setDelayUs(unsigned char delay);
void ADC0_init(long sys_clk, short vref_mv);
float ADC0_getAverage_mV(char channel, char samples);

#endif
