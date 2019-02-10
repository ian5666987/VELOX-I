// code by ntu ael

#include <stdio.h>

#include "mccif_buffer.h"

#include "Velox_beacon.h"
#include "Velox_macros.h"
#include "Velox_meta.h"
#include "Velox_screen.h"
#include "Velox_slowtask.h"
#include "Velox_uptime.h"

#include "sdcard_generic.h"
#include "sdcard_redundant.h"

#include "comm_generic.h"

#include "hash_manifest.h"

#include "obdh_auth.h"
#include "obdh_globalvariables.h"
#include "obdh_trxuv.h"

#define ONE_DAY_IN_SECOND			 86400
#define MIN_GPASS_TIME         300
#define	MAX_GPASS_TIME 				 1200 //20 minutes are the maximum amount of time for COMM to send AX25 frames (15 mins + 5 mins buffer)
#define RADIOUP_INTERVAL_MAX   6917   // at most every 7 sec
#define FERRY_INTERVAL_MIN     53
#define BEACON_INTERVAL_MIN    25     // at least 25 sec between 2 beacon
#define SD_INIT_RETRY_PERIOD   5
#define SD_INIT_RETRY_AT       3      // first try to initialize sd card is after 3 seconds
#if SD_INIT_RETRY_AT >= SD_INIT_RETRY_PERIOD
#error "at must be strickly smaller"
#endif

extern Statetask g_statetask_wireUp;
extern Statetask g_statetask_radioUp;
extern Statetask g_statetask_trxuvHk;
extern Statetask g_statetask_downlink;

unsigned short gndPass_absoluteTime = 0;

void Grace_eachSecond() {
  // BEGIN: stay alive
  g_statetask_wireUp.task.mode = timertask_repeated;
  g_statetask_radioUp.task.mode = timertask_repeated;
  g_statetask_radioUp.task.delay = min(g_statetask_radioUp.task.delay, RADIOUP_INTERVAL_MAX);
  g_statetask_downlink.task.mode = timertask_repeated; // otherwise can not authenticate
  if (g_trxuv.runtime.delay < MIN_GPASS_TIME) //this is to ensure whatever happen, we can talk to the satellite
    g_trxuv.runtime.delay = MIN_GPASS_TIME;
  if (g_slow_beacon.mode != slowtask_repeated && g_slow_beacon.mode != slowtask_disabled)
  	g_slow_beacon.mode = slowtask_repeated; //the beacon must either be off or disabled, can't be something else
  // END: stay alive

  // BEGIN: requirement by design
  g_statetask_trxuvHk.task.mode = timertask_repeated; // required to repeatedly update tx status: idle state, ...
  // END: requirement by design

  // BEGIN: probably a good idea
  // END: probably a good idea

  // --- postpone to initialize SD card to several seconds after startup, since only then SD_init() can be successful
  if (!g_sdcard.isAvailable && uptime.session % SD_INIT_RETRY_PERIOD == SD_INIT_RETRY_AT) {
    g_sdcard.isAvailable = SD_init(); // ian: expected time: first time 77ms.
    if (g_sdcard.isAvailable)
      Hash_readManifest();
  }
  // --- to ensure that after one day, the SD card is not abused by storing housekeeping data every 10 seconds ian: updated on 20140207
  if (uptime.total >= ONE_DAY_IN_SECOND && uptime.total < ONE_DAY_IN_SECOND + 5 && g_putFrame.task.delay < 60)
  	Slowtask_setPeriodic(&g_putFrame.task, 60); //to change the periodical update after one day, but only has 5 seconds window to do it (should be enough!)
  // --- ensure that beacon is not scheduled more often than BEACON_INTERVAL_MIN sec
  g_slow_beacon.delay = max(BEACON_INTERVAL_MIN, g_slow_beacon.delay);
  // ---
  g_c_beacon[XDATA_PWRS_BEACON_LENGTH] = Beacon_linear(-20, chip_meta.mcuTemperature, 85); //OBDH beacon is after the PWRS beacon, therefore XDATA_PWRS_BEACON_LENGTH
  // ---
  if ((g_us_obdh_channel_status & CHANNEL_MASK_ADS) == CHANNEL_MASK_ADS) // if ads is on, can reduce g_c_adcsBootedIn
    g_c_adcsIsReadyIn = max(0, g_c_adcsIsReadyIn - 1);
  // ---
  g_slow_gndPass.delay = max(30, g_slow_gndPass.delay); // at least half a minute -> can manually exit gnd pass anyways
  Slowtask_isReady(&g_slow_gndPass); // exit gnd pass if timeout
  // ---
  // --- To limit groundpass period to around 16 minutes
  if (g_slow_gndPass.mode == slowtask_executeOnce){
  	gndPass_absoluteTime++;
  	Runtime_fromNow(&g_trxuv.runtime); // COMM should not be turned OFF during ground pass by mistake!
  	if (gndPass_absoluteTime >= MAX_GPASS_TIME)
  		Slowtask_setDisabled(&g_slow_gndPass);
  }
  // BEGIN: manage real-time housekeeping
  if (g_slow_gndPass.mode == slowtask_disabled){ // if gnd pass is NOT active
    Slowtask_fromNow(&g_rtHk.task); // postpone rt-hk task
    gndPass_absoluteTime = 0; //keep this value as zero if the satellite is not in the groundpass
  }
  // ---
  if (Slowtask_isReady(&g_rtHk.task))
    if (g_rtHk.mask_now == 0L) // previous real-time hk queue has been processed completely
      g_rtHk.mask_now = g_rtHk.template; // trigger avalance of rtHk downlinks
  // ---
  if (g_rtHk.forceNow) { // explicit manual control from ground station = force now
    Slowtask_fromNow(&g_rtHk.task); // postpone rt-hk task
    if (1 == g_rtHk.forceNow) //only if forceNow is set to 1, then the mask_now will follow template, else it is random, as requested!
      g_rtHk.mask_now = g_rtHk.template; // trigger avalance of rtHk downlinks
    g_rtHk.forceNow = 0;
  }
  // END: manage real-time housekeeping
  g_trxuv.failures = comm_failCount;
	g_trxuv.rateCW = min(max(2, g_trxuv.rateCW), 150); // min=2 is a hw-requirement, but upper bound is debatable
  // ---
  Mccif_flush();
}
