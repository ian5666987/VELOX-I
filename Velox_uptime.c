// code by ntu ael

#include "Velox_timertask.h"
#include "Velox_uptime.h"

// local variables only visible to uptime mechanism:
Timertask uptime_timertask;
unsigned short uptime_store_count;

// function is called every "uptime_store_period" seconds to allow the subsystems to periodically store essential values:
// total uptime, battery state of charge, attitude, ...
void Uptime_storeVitals_Override();

// call with tai_time=TAI_TIME_UNAVAILABLE is absolute time is not available during initialization
void Uptime_init(unsigned long tai_time, unsigned long uptime_total, unsigned short period) {
  uptime.tai_time = tai_time;
  uptime.total = uptime_total;
  uptime.storePeriod = period;
  uptime.session = 0;
  Timertask_setPeriodic(&uptime_timertask, 1000); // the second counter is notified every 1000 ms
  uptime_store_count = 0;
}

// important: since tai_time==0 does not make sense as a time value,
// we reserve 0 as a value to indicate that absolute time has not been initialized
char Uptime_isTaiTimeValid() {
  return TAI_TIME_UNAVAILABLE != uptime.tai_time;
}

// call in main loop at every least 30 sec, ideally at least every second.
char Uptime_handler() {
  if (Timertask_isReady(&uptime_timertask)) {
    if (Uptime_isTaiTimeValid())
      ++uptime.tai_time;
    ++uptime.total;
    ++uptime.session;
    ++uptime_store_count;
    if (uptime.storePeriod < uptime_store_count) {
      uptime_store_count = 0;
      Uptime_storeVitals_Override();
    }
    return 1;
  }
  return 0;
}
