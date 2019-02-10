// code by ntu ael

#ifndef OBDH_SYSTEMCONFIG_H
#define OBDH_SYSTEMCONFIG_H

#include "C8051_oscillator.h"

// for C8051_PCA
#define EXT_PCA0MD    0x02
#define INT_PCA0MD    0x00

#define BAUDRATE_0    115200L   // UART0 baud rate in bps
#define BAUDRATE_1    115200L   // UART1 baud rate in bps

char System_getT1Prescaler(OscGear osc_gear);
char System_getCKCON(OscGear osc_gear);

#endif
