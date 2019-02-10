// code by ntu ael

//#include <stdio.h>

#include "Velox_uptime.h"

#include "rtc_generic.h"
#include "Velox_rtc_support.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_timekeeper.h"

#include "state_util.h"

char State_rtcSync(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  long absdiff;
  switch (g_timekeeper.control) {
  case timekeeper_setRtcThenRead:
    if (Uptime_isTaiTimeValid()) { // do not proceed unless uptime is valid
      Rtc_fromTai(&g_timekeeper.rtcStamp, uptime.tai_time); // set the RTC to have its TAI time
      Rtc_setRawTime(&g_timekeeper.rtcStamp);
      g_timekeeper.control = timekeeper_readRtc;
    }
    release = 1; // always release
    break; // end of timekeeper_setRtcThenRead
  case timekeeper_readRtc:
  case timekeeper_readRtcAsMaster:
    switch (*subtask) {
    case 0: // get time from RTC
      Rtc_obtainStampPut();
      Timertask_setExecuteOnce(&g_stateTask, 2);
      ++*subtask; // proceed to next subtask, state is not released
      break;
    case 1:
      if (Timertask_isReady(&g_stateTask)) { // if timeout has occurred
        if (Rtc_obtainStampGet(&g_timekeeper.rtcStamp)) { // suppose this is due to obtainRawTimePut and if the obtainRawTimeGet is successful
          g_timekeeper.taitime = Rtc_toTai(&g_timekeeper.rtcStamp);
          // ---
          if (g_timekeeper.control == timekeeper_readRtcAsMaster) {
            absdiff = uptime.tai_time - g_timekeeper.taitime;
            if (absdiff < 0L) // compute absolute value of time deviation
              absdiff = -absdiff;
            // interference with the internal clock mechanism only occurs when absolutely necessary, i.e. more than a certain minimum deviation
            // and only when data can be assumed not to be corrupted, i.e. not more than a small deviation
            if (TAITIME_DEVIATION_MIN < absdiff && absdiff < TAITIME_DEVIATION_MAX)
              uptime.tai_time = g_timekeeper.taitime;
          }
        } else
          ++g_timekeeper.failcount;
        release = 1; // release state
      }
      break;
    }
    break; // end of timekeeper_readRtc
  default:
    release = 1;
    break;
  }
  return release;
}

