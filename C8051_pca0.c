// code by ntu ael

#include <c8051F120.h>

#include "C8051_pca0.h"

// PreCondition:    Declare a function to initialize each channel of PCA0 in main code
// Input:           p_PCA0MD: PCA0 Mode setting
//                  (*pca_cpm_config)(void): function to initialize each channel of PCA0
// Overview:        This function will be executed to initialize PCA0.
void PCA0_init(char p_PCA0MD, char (*pca_cpm_config)(void)) {
  char SFRPAGE_SAVE = SFRPAGE;
  char ei;
  SFRPAGE = PCA0_PAGE;
  // Stop counter; clear all flags
  PCA0CN = 0x00; // Refer to page 335
  PCA0MD = p_PCA0MD; // Refer to page 336

  ei = pca_cpm_config();

  //Start PCA counter
  CR = 1;

  //Enable PCA interrupts
  if (ei)
    EIE1 |= 0x08;
  else
    EIE1 &= 0xF7;
  // EIP1 &= 0xF7; //set PCA counter to be low priority
  SFRPAGE = SFRPAGE_SAVE;
}

// Input:           channel: channel of PCA0 to be configured
//                  config: configuration for PCA0CPMx (x is the channel)
// Overview:        This function will be executed to set PCA0 Capture/Compare Mode for specified channel.
void PCA0_setCPM(char channel, char config) {
  switch (channel) {
  case 0:
    PCA0CPM0 = config;
    break;
  case 1:
    PCA0CPM1 = config;
    break;
  case 2:
    PCA0CPM2 = config;
    break;
  case 3:
    PCA0CPM3 = config;
    break;
  case 4:
    PCA0CPM4 = config;
    break;
  case 5:
    PCA0CPM5 = config;
    break;
  }
}

#define PCA0_REGISTER_UPDATE(n) \
  if (value == 0) { \
    PCA0CPM##n &= 0xBD; \
  } else { \
    PCA0CPM##n |= 0x42; \
    PCA0CPH##n = - value - 1; \
  }

// PreCondition:    Declare a function to initialize each channel of PCA0 in main code
// Input:           channel: channel of PCA0 to be configured
//                  value: duty cycle to be updated
// Overview:        This function will be executed to set PCA0 duty cycle for specified channel.
void PCA0_setDutyCycle(char channel, char value) {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = PCA0_PAGE;
  switch (channel) {
  case 0:
    PCA0_REGISTER_UPDATE(0)
    break;
  case 1:
    PCA0_REGISTER_UPDATE(1)
    break;
  case 2:
    PCA0_REGISTER_UPDATE(2)
    break;
  case 3:
    PCA0_REGISTER_UPDATE(3)
    break;
  case 4:
    PCA0_REGISTER_UPDATE(4)
    break;
  case 5:
    PCA0_REGISTER_UPDATE(5)
    break;
  }
  SFRPAGE = SFRPAGE_SAVE;
}

