// code by ntu ael

#ifndef OBDH_TRXUV_H
#define OBDH_TRXUV_H

#include "Velox_runtime.h"

enum enum_rateAx25 {
  rate1200, //
  rate2400, //
  rate4800, //
  rate9600,
};

struct struct_trxuv { // 2 + 10 + 1 + 1 + 2 + 6 = 22
  short failures;
  short delayMap[5];
  char autoIdleActive;
  enum enum_rateAx25 rateAx25;
  short rateCW; // 1, 2, ... 1200   dot = 600/rateCW
  Runtime runtime;
};

extern struct struct_trxuv g_trxuv;

void Trxuv_init();

char Trxuv_isAvailable();
char Trxuv_isIdleActive();
char Trxuv_getTxRateFactor();


#endif
