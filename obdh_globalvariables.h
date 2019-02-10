// code by ntu ael

#ifndef OBDH_GLOBALVARIABLES_H
#define OBDH_GLOBALVARIABLES_H

#include "Velox_state.h"
#include "Velox_slowtask.h"

#include "shared_general.h"
#include "shared_cqt.h"
#include "shared_adcs.h"
#include "shared_pwrs.h"
#include "shared_obdh.h"

#include "obdh_constant.h" // for sendAck
#include "obdh_schedule.h"

#include "hash_util.h"

#include "sdcard_generic.h"

#define CONCAT_BEACONPARAM_LENGTH     (XDATA_PWRS_BEACON_LENGTH + XDATA_OBDH_BEACON_LENGTH + XDATA_ADCS_BEACON_LENGTH)

struct struct_rtHk { // 3 + 4 + 1 + 4 + 1 = 13
  Slowtask task;
  unsigned long template;
  char forceNow;
  unsigned long mask_now;
  char hasNext; // private flag to signal from getXdat -> downlink that state is ready
};

extern ScheduleItem schedule[SCHEDULE_SIZE_LIMIT];

// ---

extern struct struct_rtHk g_rtHk;

extern unsigned short g_us_obdh_channel_status;
extern unsigned short sd_card_store_failures;
extern SdConfig g_sdConfig;
extern Slowtask g_slow_gndPass;

extern char g_c_beacon[CONCAT_BEACONPARAM_LENGTH];

extern char g_c_adcsIsReadyIn; // unit=sec

extern struct struct_putFrame g_putFrame;

extern Slowtask g_slow_beacon;
extern char g_c_sendAck_flag;
extern struct struct_sendAck g_sendAck;

extern char isRadioUp;
extern char g_c_byPassPowerCheckOnComm;
extern char xdata_txbuffer[FRAME_LENGTH_MAX];
extern char i2c_rxbuffer[FRAME_LENGTH_MAX];
extern char i2c_txbuffer[FRAME_LENGTH_MAX];
extern char readbackcopy[FRAME_LENGTH_MAX];
extern char txframeRelay[FRAME_LENGTH_MAX];

#endif
