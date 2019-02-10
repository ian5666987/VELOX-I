// code by ntu ael

#include <string.h>

#include "Velox_uptime.h"
#include "Velox_calendar.h"

#include "mccif_xdata.h"

#include "shared_obdh.h"

#include "obdh_timekeeper.h"

void Timekeeper_init() {
  memset(&g_timekeeper, 0, sizeof g_timekeeper);
  g_timekeeper.control = timekeeper_setRtcThenRead; // gnd station can switch to readAsMaster mode
}

// to be called every second
void Timekeeper_handler() {
  switch (g_timekeeper.control) {
  default:
    break;
  }
}
