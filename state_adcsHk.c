// code by ntu ael

#include <string.h>

#include "Velox_beacon.h"
#include "Velox_uptime.h"

#include "shared_general.h"
#include "shared_obdh.h"
#include "shared_adcs.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"

#include "state_util.h"

#define OBDH_ADCS_BEACON_OFFSET      XDATA_OBDH_BEACON_LENGTH + XDATA_PWRS_BEACON_LENGTH

// fails-fast if adcs is powered down
char State_adcsHk(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  short length;
  unsigned long adcs_taitime;
  long absdiff;
  char update;
  switch (*subtask) {
  case 0:
    Beacon_setUnassigned(&g_c_beacon[OBDH_ADCS_BEACON_OFFSET], XDATA_ADCS_BEACON_LENGTH);
    if (Subsystem_isAvailable(ADCS_ADDRESS)) {
      StateUtil_pokeRequestXdata(ADCS_ADDRESS, XDATA_ADCS_BEACON_OFFSET, XDATA_ADCS_BEACON_LENGTH);
      ++*subtask;
    } else
      release = 1;
    break;
  case 1:
    if (StateUtil_peek(&length, &release) == reply_success) {
      StateUtil_copyReply(&g_c_beacon[OBDH_ADCS_BEACON_OFFSET], XDATA_ADCS_BEACON_LENGTH);
      // ---
      if (Uptime_isTaiTimeValid()) { // only if obdh has time, we have any hope to update time on adcs
        StateUtil_pokeRequestXdata(ADCS_ADDRESS, XDATA_ADCS_TAITIME_OFFSET, sizeof uptime.tai_time);
        ++*subtask;
      } else
        release = 1;
    }
    break;
  case 2:
    if (StateUtil_peek(&length, &release) == reply_success) {
      StateUtil_copyReply(&adcs_taitime, sizeof adcs_taitime);
      update = adcs_taitime == TAI_TIME_UNAVAILABLE;
      if (!update) {
        absdiff = uptime.tai_time - adcs_taitime;
        if (absdiff < 0L) // compute absolute value of time deviation
          absdiff = -absdiff;
        update = TAITIME_DEVIATION_MIN < absdiff && absdiff < TAITIME_DEVIATION_MAX;
      }
      // ---
      if (update) {
        StateUtil_pokeSetXdata(ADCS_ADDRESS, XDATA_ADCS_TAITIME_OFFSET, sizeof uptime.tai_time, &uptime.tai_time); // write obdh taitime to adcs
        ++*subtask;
      } else
        release = 1;
    }
    break;
  case 3:
    if (StateUtil_peek(&length, &release) == reply_success) // time has been updated on adcs
      release = 1;
    break;
  }
  return release;
}
