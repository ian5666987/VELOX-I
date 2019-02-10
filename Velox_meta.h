// code by ntu ael

#ifndef VELOX_META_H
#define VELOX_META_H

#include "C8051_oscillator.h"

struct struct_meta { // 1 + 4 + 4 + 1 + 2 + 4 + 27 + 2 + 1 + 1 + 2 + 1 = 50 B
  OscType oscillator; // 1
  long systemFreq; // 4
  long totalResets; // 4
  char resetSource; // 1
  short spi0_failures; // 2
  float mcuTemperature; // 4
  char smbus_statusCount[27]; // 27
  unsigned short smbus_timeoutCount; // 2
  char uart0_failureCount; // 1
  char uart1_failureCount; // 1
  short watchdogCount; // 2
  char sfr_depthStatus; // 1
};

#define DECLARE_META_GLOBALVARIABLES_AT(x) \
    struct struct_meta xdata chip_meta _at_ (x);

extern struct struct_meta chip_meta;

void Meta_init();
void Meta_handler();

#endif
