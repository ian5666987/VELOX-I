// code by ntu ael

#include <c8051F120.h>

#include "C8051_delay.h"

// Refer to page 62, ADC0 Conversion Clock should be less than or equal to 2.5MHz
#define SAR_CLK     	2500000L
#define SAR_CLK_SLOW  750000L //ian 2013-07-22: this is the maximum to compensate 3,062,500 Hz (gear0) clock

sfr16 ADC0 = 0xBE; // ADC0 data register

volatile float adc0_to_mV = 0.0; // deliberately set to 0 to ensure init function is called; adc0_to_mV is set in init function
volatile unsigned char adc0_delay = 10; // value can be set via ADC0_setDelayUs, pg 59, 72.

void ADC0_setDelayUs(unsigned char delay) {
  adc0_delay = delay;
}

// Input:           sys_clk: system clock of the controller
//                  p_ref_mv: voltage reference of the controller
// Overview:        This function will be executed to configure and initialize ADC0.
void ADC0_init(long sys_clk, short vref_mv) {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = ADC0_PAGE;
  //ADC0 disabled; normal tracking mode; ADC0 conversions are initiated every write of '1' to AD0BUSY; ADC0 data is right-justified
  ADC0CN = 0x40; // Refer to page 63
  // Enable internal temperature  sensor, ADC bias generator, internal reference buffer
  REF0CN = 0x07; // Refer to page 114
  // AIN inputs are independent single-ended inputs (default)
  AMX0CF = 0x00; // Refer to page 60
  // Select internal temperature sensor as the ADC mux input
  AMX0SL = 0x0F; // Refer to page 61
  // ADC conversion clock = 2.5MHz, Internal Amplifier Gain (PGA) = 1 (default);
  if (sys_clk > 12000000L)
  	ADC0CF = ((sys_clk / SAR_CLK / 2) - 1) << 3; // Refer to page 62
  else
  	ADC0CF = ((sys_clk / SAR_CLK_SLOW / 2) - 1) << 3; // Refer to page 62
  // Enable ADC
  AD0EN = 1;
  SFRPAGE = SFRPAGE_SAVE;
  adc0_to_mV = vref_mv * 2.44140625e-4; // 2.44140625e-004 == 1 / 4096;
  ADC0_setDelayUs(10);
}

// Input:           channel: channel select for ADC0 MUX
// Output:          l_s_value: ADC0 Data Word
// Overview:        This function will be executed to convert the voltage at the selected ADC0 channel input and read
//                  ADC0 data value which is between 0x0000 and 0x0fff. (Refer to page 65)
short ADC0_readShort(char channel) {
  char SFRPAGE_SAVE = SFRPAGE;
  short value;
  SFRPAGE = ADC0_PAGE; // Switch to ADC0 page
  AMX0SL = channel; // Select the ADC0 MUX input
  // An absolute minimum settling time of 1.5 micro sec (us) is required after any MUX or PGA selection.
  Delay_us(adc0_delay); // page 77: The required settling time for a given settling accuracy (SA) may be approximated by Equation 6.1.
  AD0INT = 0; // Step 1: Clear AD0INT
  AD0BUSY = 1; // Step 2: Set AD0BUSY
  while (!AD0INT) { // Step 3: Poll ADOINT for true
  }
  value = ADC0; // Store conversion results
  AD0INT = 0; // Clear ADC conversion complete indicator
  SFRPAGE = SFRPAGE_SAVE; // Restore the SFRPAGE
  return value;
}

// Input:           channel: the channel value for AIN0
//                  no_of_samples: number of samples to get for average value
// Output:          sum * adc0_to_mV / samples = average value of samples
// Overview:        This function will be executed to get the average value based on the number of samples given.
float ADC0_getAverage_mV(char channel, char samples) {
  char count;
  long sum = 0;
  for (count = 0; count < samples; ++count)
    sum += ADC0_readShort(channel);
  return sum * adc0_to_mV / samples; // conversion is the same for all ADC0; refer to page 65
}
