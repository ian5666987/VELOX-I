// code by ntu ael

#ifndef OBDH_TIMEKEEPER_H
#define OBDH_TIMEKEEPER_H

#include "rtc_generic.h"
#include "Velox_calendar.h"

enum enum_timekeeperControl {
  timekeeper_idle, //
  timekeeper_setRtcThenRead, //
  timekeeper_readRtc, //
  timekeeper_readRtcAsMaster, //
};

struct struct_timekeeper { // 8 + 1 + 1 + 4 = 14
  RtcStamp rtcStamp;
  enum enum_timekeeperControl control;
  char failcount;
  unsigned long taitime;
};

extern struct struct_timekeeper g_timekeeper;

void Timekeeper_init();
void Timekeeper_handler();

#endif
