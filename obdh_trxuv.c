// code by ntu ael

#include "Velox_uptime.h"

#include "shared_general.h"
#include "shared_pwrs.h"

#include "comm_itc.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_trxuv.h"

#define  COMM_STARTUP_DELAY_SEC    2L  // require pwrs on for 2+1=3 sec until contact

void Trxuv_init() {
  // generated in matlab based on time to send 1 frame of 231 B at 1200 bps in idle active mode
  g_trxuv.delayMap[0] = 5639;
  g_trxuv.delayMap[1] = 3527;
  g_trxuv.delayMap[2] = 2539;
  g_trxuv.delayMap[3] = 1409;
  g_trxuv.delayMap[4] = STATE_DONWLINK_DELAY_MIN;
  g_trxuv.autoIdleActive = 1; // automatic as default, since downlink is efficient that way, and no additional handling from gnd station is required
  g_trxuv.rateAx25 = 0; // this is reset to 0, until gnd station decides otherwise
  g_trxuv.rateCW = 100; // default is overwritten by manifest //ian 20131024: set 100 as default after meeting on 20131021 (20 is too fast!)
  // comm is reset every 16 minutes. don't increase this value, since comm might continue to send and then use a lot of power!
  g_trxuv.runtime.delay = 960; // 60 * 16 == 960 i.e. 16 minutes, stored in eeprom/SD card, so that when comm is found to be stable during the mission duration can be extended
  Runtime_fromNow(&g_trxuv.runtime);
}

char Trxuv_isAvailable() {
  return g_c_byPassPowerCheckOnComm ? COMM_STARTUP_DELAY_SEC < uptime.session : (g_us_obdh_channel_status & CHANNEL_MASK_COMM) && (COMM_STARTUP_DELAY_SEC < uptime.session);
}

char Trxuv_isIdleActive() {
  return g_itc.tx_status & COMM_IDLE_STATUS_MASK;
}

// returns 1 2 3 4, as effective speedup (not physical)
char Trxuv_getTxRateFactor() {
  unsigned char mask;
  mask = g_itc.tx_status & COMM_TX_RATE_MASK;
  mask >>= 2;
  return mask + 1; // TODO formula is subject to testing
}

