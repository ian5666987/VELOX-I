// code by ntu ael

#ifndef OBDH_CQT_H
#define OBDH_CQT_H

#include "Velox_runtime.h"
#include "Velox_timertask.h"
#include "Velox_uptime.h"

enum enum_cqtHandlerState {
  cqt_idle = 0, //
  cqt_requestPowerUp = 1, //
  cqt_powerUpGiven = 2, //
  cqt_confirmStable = 3, //
  cqt_listen = 4, //
  cqt_delayedRx = 5, //
  cqt_requestPowerDown = 6, //
  cqt_powerDownGiven = 7 //
};

struct struct_cqtInfo { // 28
  Timertask task;
  unsigned long byteCount;
  unsigned short framesRejected;
  short current; // mA
  Runtime runtime; // sec
  short haltBelow; // mA
  unsigned char txWait;
  char address;
  enum enum_cqtHandlerState state;
};

extern struct struct_cqtInfo g_cqtInfo;

void Cqt_init();
void Cqt_handler();

#endif
