// code by ntu ael

#include <string.h>

#include "Velox_beacon.h"
#include "Velox_macros.h"
#include "Velox_uptime.h"

#include "mccif_xdata.h"

#include "shared_general.h"
#include "shared_obdh.h"
#include "shared_pwrs.h"

#include "obdh_constant.h"
#include "obdh_cqt.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"
#include "obdh_trxuv.h"

#include "state_util.h"

enum enum_pwrsHk {
  pwrsHk_channelStatus = 0, // 0
  pwrsHk_beacon, //
  pwrsHk_cqtCurrent, //
  pwrsHk_cqtCurrent_ack, //
  pwrsHk_commOff_ack, //
  pwrsHk_taiTime_ack, //
};

char State_pwrsHk(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  short length;
  long pwrs_time;
  switch (*subtask) {
  case pwrsHk_channelStatus: // request channel status
    Beacon_setUnassigned(&g_c_beacon[XDATA_PWRS_BEACON_START_POS], XDATA_PWRS_BEACON_LENGTH); // the pwrs beacon location is right after the header therefore XDATA_PWRS_BEACON_START_POS
    if (Subsystem_isAvailable(PWRS_ADDRESS)) {
      // ---
      StateUtil_pokeRequestXdata(PWRS_ADDRESS, XDATA_PWRS_CHANNEL_STATUS_OFFSET, 2);
      ++*subtask;
    } else
      release = 1; // signal to release
    break;
  case pwrsHk_beacon: // wait for channel status; request beacon
    if (StateUtil_peek(&length, &release) == reply_success) {
      g_us_obdh_channel_status = extractFromBuffer(unsigned short, &i2c_rxbuffer[MCCIF_DATA_OFFSET]); // update channel status
      Recon_ChannelStatusUpdated_Event();
      // ---
      StateUtil_pokeRequestXdata(PWRS_ADDRESS, XDATA_PWRS_BEACON_OFFSET, XDATA_PWRS_BEACON_LENGTH); // request beacon
      ++*subtask;
    }
    break;
  case pwrsHk_cqtCurrent: // wait for beacon
    if (StateUtil_peek(&length, &release) == reply_success) {
      StateUtil_copyReply(&g_c_beacon[XDATA_PWRS_BEACON_START_POS], XDATA_PWRS_BEACON_LENGTH);
      // ---
      StateUtil_pokeRequestXdata(PWRS_ADDRESS, XDATA_PWRS_CQT_CURRENT, 2); // request for cqt current
      ++*subtask;
    }
    break;
  case pwrsHk_cqtCurrent_ack:
    if (StateUtil_peek(&length, &release) == reply_success) {
      StateUtil_copyReply(&g_cqtInfo.current, 2);
      if (Uptime_isTaiTimeValid()) {
        if (Runtime_hasExpired(&g_trxuv.runtime)) {
          length = CHANNEL_MASK_COMM;
          StateUtil_pokeSetXdata(PWRS_ADDRESS, XDATA_PWRS_CH_EXTREQOFF_OFFSET, 2, &length);
          *subtask = pwrsHk_commOff_ack;
        } else
          release = 1;
      } else {
        StateUtil_pokeRequestXdata(PWRS_ADDRESS, XDATA_PWRS_TAITIME_OFFSET, sizeof uptime.tai_time);
        *subtask = pwrsHk_taiTime_ack;
      }
    }
    break;
  case pwrsHk_commOff_ack:
    if (StateUtil_peek(&length, &release) == reply_success) {
      Runtime_fromNow(&g_trxuv.runtime);
      release = 1;
    }
    break;
    // ---
  case pwrsHk_taiTime_ack:
    if (StateUtil_peek(&length, &release) == reply_success) {
      pwrs_time = extractFromBuffer(unsigned long, &i2c_rxbuffer[MCCIF_DATA_OFFSET]);
      if (pwrs_time) // prevent invalid time to be set on obdh, check is redundant, since obdh_taiTime is invalid
        uptime.tai_time = pwrs_time;
      release = 1;
    }
    break;
  }
  return release;
}
