// code by ntu ael

#ifndef SHARED_OBDH_H
#define SHARED_OBDH_H

#include "shared_cqt.h"

#define XDATA_OBDH_CQT_OFFSET        100
#define XDATA_OBDH_CQTDATA_OFFSET    105
#define XDATA_OBDH_CQT_LENGTH       (5+CQT_RAWDATA) // 5 + 211 excluding 2 bytes of binary check
// next 2 bytes are reserved however !!!

// ---

#define XDATA_OBDH_SC0_OFFSET        350
#define XDATA_OBDH_SC0_LENGTH        216

#define XDATA_OBDH_SC1_OFFSET        566
#define XDATA_OBDH_SC1_LENGTH        216
#if XDATA_OBDH_SC1_OFFSET != XDATA_OBDH_SC0_OFFSET + XDATA_OBDH_SC0_LENGTH
#error "out of sync: sc1"
#endif

#define XDATA_OBDH_SC2_OFFSET        782
#define XDATA_OBDH_SC2_LENGTH        216
#if XDATA_OBDH_SC2_OFFSET != XDATA_OBDH_SC1_OFFSET + XDATA_OBDH_SC1_LENGTH
#error "out of sync: sc2"
#endif

#define XDATA_OBDH_SC3_OFFSET        998
#define XDATA_OBDH_SC3_LENGTH        216
#if XDATA_OBDH_SC3_OFFSET != XDATA_OBDH_SC2_OFFSET + XDATA_OBDH_SC2_LENGTH
#error "out of sync: sc3"
#endif

// ---

#define XDATA_OBDH_STA_OFFSET       1250
#define XDATA_OBDH_STA_LENGTH        120

// ---

#define XDATA_OBDH_MET_OFFSET       1400
#define XDATA_OBDH_MET_LENGTH        217 //WARNING: for stable COMM, never put large packet size!

#define XDATA_OBDH_ADD_OFFSET       1650
//#define XDATA_OBDH_ADD_LENGTH       20  // not required

#define XDATA_OBDH_EXT_OFFSET       1700
#define XDATA_OBDH_EXT_LENGTH         99

#define XDATA_OBDH_ACK_OFFSET       1800
#define XDATA_OBDH_ACK_LENGTH         18

#define XDATA_OBDH_PAS_OFFSET       1830
#define XDATA_OBDH_PAS_LENGTH          9

#define XDATA_OBDH_BEACON_LENGTH       1
#define XDATA_PWRS_BEACON_START_POS    0 //TOTRACE obselete parameter, but must be put to zero

#endif
