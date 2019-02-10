// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "Velox_beacon.h"

#include "sdcard_redundant.h"

#include "shared_general.h"
#include "shared_obdh.h"
#include "shared_adcs.h"
#include "shared_pwrs.h"

#include "obdh_constant.h"
#include "obdh_cqt.h"
#include "obdh_debug.h"
#include "obdh_globalvariables.h"
#include "obdh_trxuv.h"

#include "hash_blob.h"
#include "hash_frame.h"
#include "hash_util.h"

#include "state_util.h"

#define XDATASEGMENT_APPEND(sys, ofs, len) \
    xdatSeg[count].system = sys; \
    xdatSeg[count].offset = ofs; \
    xdatSeg[count].length = len; \
    ++count

extern char g_c_gearbox_switch;

XdataSegment xdata xdatSeg[XDATSEG_NUMEL];

void Constant_init() {
  char count = 0;
  XDATASEGMENT_APPEND(OBDH_ADDRESS, XDATA_OBDH_SC0_OFFSET, XDATA_OBDH_SC0_LENGTH);
  XDATASEGMENT_APPEND(OBDH_ADDRESS, XDATA_OBDH_SC1_OFFSET, XDATA_OBDH_SC1_LENGTH);
  XDATASEGMENT_APPEND(OBDH_ADDRESS, XDATA_OBDH_SC2_OFFSET, XDATA_OBDH_SC2_LENGTH);
  XDATASEGMENT_APPEND(OBDH_ADDRESS, XDATA_OBDH_SC3_OFFSET, XDATA_OBDH_SC3_LENGTH);
  XDATASEGMENT_APPEND(OBDH_ADDRESS, XDATA_OBDH_STA_OFFSET, XDATA_OBDH_STA_LENGTH);
  // --- 5
  XDATASEGMENT_APPEND(OBDH_ADDRESS, XDATA_OBDH_MET_OFFSET, XDATA_OBDH_MET_LENGTH);
  XDATASEGMENT_APPEND(OBDH_ADDRESS, XDATA_OBDH_EXT_OFFSET, XDATA_OBDH_EXT_LENGTH);
  XDATASEGMENT_APPEND(PWRS_ADDRESS, XDATA_PWRS_MET_OFFSET, XDATA_PWRS_MET_LENGTH);
  XDATASEGMENT_APPEND(PWRS_ADDRESS, XDATA_PWRS_CAL_OFFSET, XDATA_PWRS_CAL_LENGTH);
  XDATASEGMENT_APPEND(PWRS_ADDRESS, XDATA_PWRS_ETC_OFFSET, XDATA_PWRS_ETC_LENGTH);
  // --- 10
  XDATASEGMENT_APPEND(PWRS_ADDRESS, XDATA_PWRS_HK_OFFSET, XDATA_PWRS_HK_LENGTH);
  XDATASEGMENT_APPEND(PWRS_ADDRESS, XDATA_PWRS_PAY_OFFSET, XDATA_PWRS_PAY_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_HK_OFFSET, XDATA_ADCS_HK_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_MET_OFFSET, XDATA_ADCS_MET_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_SUNSENSOR1_OFFSET, XDATA_ADCS_SUNSENSOR1_LENGTH);
  // --- 15
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_SUNSENSOR2_OFFSET, XDATA_ADCS_SUNSENSOR2_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_IMU_OFFSET, XDATA_ADCS_IMU_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_ADS_OFFSET, XDATA_ADCS_ADS_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_EKF_OFFSET, XDATA_ADCS_EKF_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_ACT_OFFSET, XDATA_ADCS_ACT_LENGTH);
  // --- 20
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_ACS_OFFSET, XDATA_ADCS_ACS_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_ACS2_OFFSET, XDATA_ADCS_ACS2_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_ACS3_OFFSET, XDATA_ADCS_ACS3_LENGTH);
  XDATASEGMENT_APPEND(ADCS_ADDRESS, XDATA_ADCS_GPS_OFFSET, XDATA_ADCS_GPS_LENGTH);
  // --- 24
#if XDATSEG_NUMEL != 24 // total
#error "out of sync: XDATSEG_NUMEL"
#endif
  // ---
  memset(&g_sdcard, 0, sizeof g_sdcard);
  // ---
  g_hashFrame.offset = 0x030000;
  g_hashFrame.pages = 0x020000; //default value changes to 0x020000 ian: updated on 20140207, after agreement by the team members
  g_hashFrame.stride = XDATSEG_NUMEL;
  g_hashFrame.copies = 1;
  // ---
  Slowtask_setPeriodic(&g_putFrame.task, 10); // every 10 seconds at first, and changed to every minute after one day
  g_putFrame.putIterator = putIterator_inactive;
  g_putFrame.frameIndex.page = 0;
  g_putFrame.frameIndex.index = -1; // -1 is an invalid index, but will be set correctly by xdatSegGet
  g_putFrame.mask = WOD_DEFAULT_MASK;  //ian 20131024: set this as default after meeting on 20131021
  // ---
  g_getFrame.frameIndex.page = 0; // 0 would be the first page
  g_getFrame.frameIndex.index = 0; // 0 would be the first to send, if mask has first bit set
  g_getFrame.mask = 1L << XDATSEG_NUMEL;
  --g_getFrame.mask;
  g_getFrame.stride = 1;
  g_getFrame.numPages = 0;
  // ---
  g_pinFrame.frameIndex.page = 0;
  g_pinFrame.frameIndex.index = XDATSEG_NUMEL; // indicates that no downlink is desired
  g_pinFrame.mask = 0;
  // ---
  g_intercom.timeout = 250; // ms
  g_intercom.invalids = 0;
  g_intercom.timeouts = 0;
  g_intercom.lastFailedSystem = 0;
  // ---
  g_manifest.offset = 0x10000; // small numbers, e.g. 0x100 have produced errors
  g_manifest.divider = 90; // not used
  g_manifest.copies = 5; // During development, think about friendly way (put 2), during flight phase, be more secure (put 5)
  g_manifest.failures = 0;
  // ---
  g_c_sendAck_flag = 0;
  g_sendAck.uplinkId = -1;
  // ---
  g_hashBlob.offset = 0x330000;
  g_hashBlob.copies = 1;
  // ---
  g_getBlob.index = 0;
  g_getBlob.type = 1;
  g_getBlob.head = 0;
  g_getBlob.tail = 0;
  memset(g_getBlob.lo, -1, sizeof g_getBlob.lo);
  memset(g_getBlob.hi, -1, sizeof g_getBlob.hi);
  // ---
  memset(&g_gndCommand, 0, sizeof g_gndCommand);
  // ---
  g_us_obdh_channel_status = 0;
  g_c_adcsIsReadyIn = 2;
  // ---
  memset(&g_rtHk, 0, sizeof g_rtHk); // volatile mask = 0; hasNext = 0;
  Slowtask_setRepeated(&g_rtHk.task, 30); //ian 20131024: change the default from 60 seconds to 30 seconds after meeting on 20131021
  g_rtHk.template = HK_DEFAULT_MASK; //ian 20131024: set this as default after meeting on 20131021
  // ---
  g_slow_gndPass.delay = 180; // ian: the groundpass here is now set as 3 minutes before it escape
  Slowtask_setDisabled(&g_slow_gndPass);
  // ---
  Slowtask_setRepeated(&g_slow_beacon, 60); //ian 20131024: change the default from 240 to 60 after meeting on 20131021
  // ---
  Beacon_setUnassigned(g_c_beacon, sizeof g_c_beacon);
  // ---
  g_c_gearbox_switch = GEAR_NO_CHANGE;
  // ---
  sd_card_store_failures = 0;
  // --- ian 20140310: add this to distinguish between radioUplink and wireUplink
  isRadioUp = 0; //radio uplink will have radio downlink acknowledgment. Otherwise, don't send radio downlink!
  g_c_byPassPowerCheckOnComm = 0; //put the default as not by-passing
}
