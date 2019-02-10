// code by ntu ael

#ifndef SHARED_PWRS_H
#define SHARED_PWRS_H

// serves as a criteria to check if any subsystem is switched on
#define  CHANNEL_MASK_ADS           0x001
#define  CHANNEL_MASK_ACS           0x002
#define  CHANNEL_MASK_COMM          0x004
#define  CHANNEL_MASK_DEPL          0x008

#define  CHANNEL_MASK_CQT           0x010
// #define CHANNEL_MASK_PROBE       0x020 // bit reserved for Probe, defined in load_probe.c , previous name CHANNEL_MASK_3V3CQT, name still prevails in mcc...
#define  CHANNEL_MASK_CSFB          0x040
#define  CHANNEL_MASK_SBC           0x080

#define  CHANNEL_MASK_XBEE          0x100
#define  CHANNEL_MASK_BE            0x200
#define  CHANNEL_MASK_TS            0x400
#define  CHANNEL_MASK_HEATER        0x800

// ---

#define XDATA_PWRS_CAL_OFFSET 100
#define XDATA_PWRS_CAL_LENGTH 153

#define XDATA_PWRS_CONFIG_LENGTH 101

#define CAMERA_DATA_LENGTH 220

#define XDATA_PWRS_BIN_OFFSET 300
#define XDATA_PWRS_BIN_LENGTH (CAMERA_DATA_LENGTH+5)

#define XDATA_PWRS_MET_OFFSET 550
#define XDATA_PWRS_MET_LENGTH 107

#define XDATA_PWRS_CH_EXTREQON_OFFSET   611
#define XDATA_PWRS_CH_EXTREQOFF_OFFSET  613

#define XDATA_PWRS_ETC_OFFSET 700
#define XDATA_PWRS_TSK_OFFSET 791
#define XDATA_PWRS_ETC_LENGTH 170

#define XDATA_PWRS_BEACON_OFFSET  (XDATA_PWRS_TSK_OFFSET+66)  // shared with obdh
#define XDATA_PWRS_BEACON_LENGTH  13

#define XDATA_PWRS_HK_OFFSET 900
#define XDATA_PWRS_HK_LENGTH 163

#define XDATA_PWRS_CQT_CURRENT  976

#define XDATA_PWRS_TAITIME_OFFSET   XDATA_PWRS_HK_OFFSET

#define XDATA_PWRS_CHANNEL_STATUS_OFFSET      (XDATA_PWRS_HK_OFFSET + 107)

// N-Sat XBee variables
// PAY contains XBP (xbee config) and CCT (camera control)
#define XDATA_PWRS_PAY_OFFSET 1100
#define XDATA_PWRS_PAY_LENGTH 87


#define XDATA_XBEE_TO_OBDH_READY  (XDATA_PWRS_PAY_OFFSET + 0)  // shared with obdh
#define XDATA_PWRS_CAM_AVAILABLE (XDATA_PWRS_PAY_OFFSET + 60)  // shared with obdh
#define XDATA_PWRS_XBEE_OP (XDATA_PWRS_PAY_OFFSET + 79)  // shared with obdh
// TODO this offset is only temporary until can merge with other parameters
// ---------------------------

// COLLECTION ONLY DURING XBEE/PSAT SEPARATION PHASE
#define XDATA_PWRS_XBE_OFFSET 1200
#define XDATA_PWRS_XBE_LENGTH 81 // + XDATA_PWRS_PST_LENGTH // 50+31 (xbee length + psat length)

//#define XDATA_PWRS_PST_OFFSET PWRS_XBEE_GLOBAL_VARIABLES_LAST_OFFSET
#define XDATA_PWRS_PST_LENGTH 31

// TO store P-SAT XBee variables on N-SAT
// COLLECTION ONLY DURING XBEE/PSAT SEPARATION PHASE

#endif
