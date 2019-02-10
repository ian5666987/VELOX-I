// code by ntu ael

#include "C8051_adc.h"

// Vtemp = (2.86 mV/'C) * Temp + 776 mV // Refer to AN103 or page 74 in datasheet
float Mcutemp_inCelsius() {
  float adc_mV;
  adc_mV = ADC0_getAverage_mV(8, 2);
  return (adc_mV - 776.0) / 2.86;
}
