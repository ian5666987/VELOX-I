// code by ntu ael

#include <c8051F120.h>

#include "Velox_meta.h"
#include "Velox_runtime.h"
#include "Velox_uptime.h"

#include "Mccif_signal.h"

#include "Velox_screen.h"
#include "Velox_rtc_support.h"

#include "comm_generic.h"
#include "comm_imc.h"
#include "comm_itc.h"
#include "sdcard_redundant.h"

#include "shared_obdh.h"
#include "shared_general.h"

#include "obdh_auth.h"
#include "obdh_cqt.h"
#include "obdh_debug.h"
#include "obdh_globalvariables.h"
#include "obdh_timekeeper.h"
#include "obdh_trxuv.h"

#include "hash_blob.h"
#include "hash_frame.h"
#include "hash_util.h"

#include "state_util.h"

short xdata g_s_BinaryId _at_ (XDATA_OBDH_CQT_OFFSET + 0);
char xdata g_c_BinaryType _at_ (XDATA_OBDH_CQT_OFFSET + 2); // for cqt, this has to == blobtype_cqt == 4
unsigned short xdata g_us_BinaryFrame _at_ (XDATA_OBDH_CQT_OFFSET + 3);
char xdata g_c_BinaryContent[CQT_RAWDATA] _at_ (XDATA_OBDH_CQT_OFFSET + 5);
short xdata g_us_BinaryCheck _at_ (XDATA_OBDH_CQT_OFFSET + 216); // this will not be sent as part of the xdata packet (!)

// ---

ScheduleItem xdata schedule[SCHEDULE_SIZE_LIMIT] _at_ (XDATA_OBDH_SC0_OFFSET);

// ---

Statetask xdata g_statetask_pwrsHk _at_ (XDATA_OBDH_STA_OFFSET + 0);
Statetask xdata g_statetask_adcsHk _at_ (XDATA_OBDH_STA_OFFSET + 6);
Statetask xdata g_statetask_trxuvHk _at_ (XDATA_OBDH_STA_OFFSET + 12);
Statetask xdata g_statetask_xdatSegGet _at_ (XDATA_OBDH_STA_OFFSET + 18);
Statetask xdata g_statetask_wireUp _at_ (XDATA_OBDH_STA_OFFSET + 24);
Statetask xdata g_statetask_radioUp _at_ (XDATA_OBDH_STA_OFFSET + 30);
Statetask xdata g_statetask_cmdHandler _at_ (XDATA_OBDH_STA_OFFSET + 36);
Statetask xdata g_statetask_scheduler _at_ (XDATA_OBDH_STA_OFFSET + 42);
Statetask xdata g_statetask_downlink _at_ (XDATA_OBDH_STA_OFFSET + 48);
Statetask xdata g_statetask_ferryCam _at_ (XDATA_OBDH_STA_OFFSET + 54);
Statetask xdata g_statetask_ferryXBee _at_ (XDATA_OBDH_STA_OFFSET + 60);
Statetask xdata g_statetask_ferryGps _at_ (XDATA_OBDH_STA_OFFSET + 66);
Statetask xdata g_statetask_rtcSync _at_ (XDATA_OBDH_STA_OFFSET + 72);
Statetask xdata g_statetask_powerCqt _at_ (XDATA_OBDH_STA_OFFSET + 78);
State xdata mainState _at_ (XDATA_OBDH_STA_OFFSET + 84);
struct struct_trxuv xdata g_trxuv _at_ (XDATA_OBDH_STA_OFFSET + 93); //+22 (which affect the rests of the position)
Timertask xdata g_task_mccif _at_ (XDATA_OBDH_STA_OFFSET + 115);

#if 115 + 5 != XDATA_OBDH_STA_LENGTH
#error "out of sync: sta length"
#endif

// ---

DECLARE_UPTIME_GLOBALVARIABLES_AT(XDATA_OBDH_MET_OFFSET + 0)

DECLARE_META_GLOBALVARIABLES_AT(XDATA_OBDH_MET_OFFSET + 14)
// +50

struct struct_sdcard xdata g_sdcard _at_ (XDATA_OBDH_MET_OFFSET + 64); // +15
struct struct_hashFrame xdata g_hashFrame _at_ (XDATA_OBDH_MET_OFFSET + 79); // +10 (which affect the rests of the position)
struct struct_hashBlob xdata g_hashBlob _at_ (XDATA_OBDH_MET_OFFSET + 89); // +5
struct struct_getBlob xdata g_getBlob _at_ (XDATA_OBDH_MET_OFFSET + 94); // +26

unsigned short xdata g_us_obdh_channel_status _at_ (XDATA_OBDH_MET_OFFSET + 120);
unsigned short xdata sd_card_store_failures _at_ (XDATA_OBDH_MET_OFFSET + 122);

Slowtask xdata g_slow_gndPass _at_ (XDATA_OBDH_MET_OFFSET + 124);
struct struct_gndCommand xdata g_gndCommand _at_ (XDATA_OBDH_MET_OFFSET + 127); // +4
Slowtask xdata g_slow_beacon _at_ (XDATA_OBDH_MET_OFFSET + 131);
char xdata g_c_beacon[CONCAT_BEACONPARAM_LENGTH] _at_ (XDATA_OBDH_MET_OFFSET + 134); // keep beacon array at the end of packet

struct struct_rtHk xdata g_rtHk _at_ (XDATA_OBDH_MET_OFFSET + 134 + CONCAT_BEACONPARAM_LENGTH); //+13
char xdata g_c_gearbox_switch _at_ (XDATA_OBDH_MET_OFFSET + 147 + CONCAT_BEACONPARAM_LENGTH); //+1

struct struct_imc xdata g_imc _at_ (XDATA_OBDH_MET_OFFSET + 148 + CONCAT_BEACONPARAM_LENGTH); // + 19
struct struct_itc xdata g_itc _at_ (XDATA_OBDH_MET_OFFSET + 167 + CONCAT_BEACONPARAM_LENGTH); // + 5

struct struct_putFrame xdata g_putFrame _at_ (XDATA_OBDH_MET_OFFSET + 172 + CONCAT_BEACONPARAM_LENGTH); // + 13 (following the change, which affects the rests of the position)
struct struct_getFrame xdata g_getFrame _at_ (XDATA_OBDH_MET_OFFSET + 185 + CONCAT_BEACONPARAM_LENGTH); // + 12 (following the change, which affects the rests of the position)

#if XDATA_OBDH_MET_LENGTH != 197 + CONCAT_BEACONPARAM_LENGTH
#error "out of sync: met length"
#endif

// ---

struct struct_scheduleAdd xdata g_scheduleAdd _at_ (XDATA_OBDH_ADD_OFFSET); // +20

// ---

struct struct_screen xdata screen _at_ (XDATA_OBDH_EXT_OFFSET + 0); // + 10
struct struct_pinFrame xdata g_pinFrame _at_ (XDATA_OBDH_EXT_OFFSET + 10); // + 9 (following the change, which affects the rests of the position)
struct struct_manifest xdata g_manifest _at_ (XDATA_OBDH_EXT_OFFSET + 19); // + 8
struct struct_intercom xdata g_intercom _at_ (XDATA_OBDH_EXT_OFFSET + 27); // + 7
MccifSignal xdata g_obdh_mccifSignal _at_ (XDATA_OBDH_EXT_OFFSET + 34); // + 15 COMM data is gone (24 bytes)
char xdata g_c_sendAck_flag _at_ (XDATA_OBDH_EXT_OFFSET + 49); // + 1
char xdata g_c_adcsIsReadyIn _at_ (XDATA_OBDH_EXT_OFFSET + 50); // + 1; // unit = sec
struct struct_timekeeper xdata g_timekeeper _at_ (XDATA_OBDH_EXT_OFFSET + 51); // + 14
struct struct_cqtInfo xdata g_cqtInfo _at_ (XDATA_OBDH_EXT_OFFSET + 65); // +24
char xdata g_c_byPassPowerCheckOnComm _at_ (XDATA_OBDH_EXT_OFFSET + 89);
SdConfig xdata g_sdConfig _at_ (XDATA_OBDH_EXT_OFFSET + 90); //+9

#if 99 != XDATA_OBDH_EXT_LENGTH
#error "out of sync: ext length"
#endif

// ---

struct struct_sendAck xdata g_sendAck _at_ (XDATA_OBDH_ACK_OFFSET); // + 16

struct struct_auth xdata g_auth _at_ (XDATA_OBDH_PAS_OFFSET);

char xdata isRadioUp;
char xdata xdata_txbuffer[FRAME_LENGTH_MAX]; // for instance to compile a frame with local memory for debug monitoring
char xdata i2c_rxbuffer[FRAME_LENGTH_MAX]; // receives housekeeping data from other subsystems
char xdata i2c_txbuffer[FRAME_LENGTH_MAX]; // sending requests for hk data
char xdata readbackcopy[FRAME_LENGTH_MAX]; // for immediate verification of sd card write operation
char xdata txframeRelay[FRAME_LENGTH_MAX]; // for state xdatSeg to transfer a frame for downlink state
