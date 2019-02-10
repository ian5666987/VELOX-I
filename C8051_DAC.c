// code by ntu ael

#include <c8051F120.h>

#include "C8051_DAC.h"

// the digital-to-analog converter is set to 8 bit resolution (12 bit are possible)
// Overview:        This function will be executed to configure and initialize DAC0.
void DAC0_init() {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = DAC0_PAGE;
  // enable DAC0, updated on write to DAC0H, values in left-justified mode: 0xfff0
  DAC0CN = 0x84; // Refer to page 108
  DAC0L = 0; // reset value of DAC0L is 0
  SFRPAGE = SFRPAGE_SAVE;
}

// Overview:        This function will be executed to configure and initialize DAC1.
void DAC1_init() {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = DAC1_PAGE;
  // enable DAC1, updated on write to DAC1H, values in left-justified mode: 0xfff0
  DAC1CN = 0x84; // Refer to page 110
  DAC1L = 0; // reset value of DAC1L is 0
  SFRPAGE = SFRPAGE_SAVE;
}

// Input:           value: value to be set into DAC0H
// Overview:        This function will be executed to set DAC0H to desired value.
void DAC0_setRaw(char value) {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = DAC0_PAGE;
  DAC0H = value;
  SFRPAGE = SFRPAGE_SAVE;
}

// Input:           value: value to be set into DAC1H
// Overview:        This function will be executed to set DAC1H to desired value.
void DAC1_setRaw(char value) {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = DAC1_PAGE;
  DAC1H = value;
  SFRPAGE = SFRPAGE_SAVE;
}
