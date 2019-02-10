// code by ntu ael

#include <c8051F120.h>

#include "C8051_oscillator.h"
#include "C8051_reset.h"

#include "shared_general.h"
#include "shared_obdh.h"

#include "Velox_rtc_support.h"
#include "Velox_calendar.h"
#include "Velox_watchdog.h"

#include "comm_imc.h"
#include "comm_itc.h"

#include "obdh_constant.h"
#include "obdh_debug.h"
#include "obdh_globalvariables.h"
#include "obdh_timekeeper.h"
#include "obdh_systeminit.h"

#include "hash_manifest.h"

#include "sdcard_generic.h"
#include "sdcard_redundant.h"

extern char g_c_gearbox_switch;

char State_cmdHandler(char* subtask) {
  char dummy_buffer[24];
  switch (g_gndCommand.id) {
  case 0:
    break;
  case 1: // imc ---
    Comm_rxReset();
    break;
  case 2: // itc ---
    Comm_txReset();
    break;
#define   CMDID_BITRATE   3
  case CMDID_BITRATE + 0: // itc
  case CMDID_BITRATE + 1:
  case CMDID_BITRATE + 2:
  case CMDID_BITRATE + 3:
    Comm_setTxBitrate(g_gndCommand.id - CMDID_BITRATE); // 0 -> 1200, 1 -> 2400, 2 -> 4800, 3 -> 9600
    break;
  case 7: // itc
    // tx_mode == 0x00, or tx_mode == 0x03 are not allowed since would disable uplink/downlink capability
    Comm_setTxOutputMode(0x1); // 0x01 - nominal telemetry mode
    break;
  case 8: // itc
    Comm_setTxCwCharRate(g_gndCommand.parameter); // rate = 600/value
    break;
    // ---
  case 31:
    if (!SD_writeSingleBlock(g_gndCommand.parameter, dummy_buffer, sizeof dummy_buffer))
      g_sdcard.write.failures++;
    g_sdcard.write.count++;
    break;
  case 32:
    if (!SD_readSingleBlock(g_gndCommand.parameter, dummy_buffer, sizeof dummy_buffer))
      g_sdcard.read.failures++;
    g_sdcard.read.count++;
    break;
  case 35: //ian: updated on 20140207 - to support correct initialization
  	Hash_writeManifest();
    break;
  case 53: //ian: updated on 20140207 - to support correct initialization
  	Hash_readManifest();
    break;
  case 0x70:
    C8051_INTERNAL_RESET
    break;
  case 0x71:
    C8051_POWER_ON_RESET
    break;
  case COMMAND_WATCHDOG_TEST:
    EA = 1;
    Watchdog_disable();
    while (1)
      ; // test watchdog mechanism
    break;
  }
  g_gndCommand.id = 0;
  // ---
  // switching gears is in state to "guarantee" that no i2c communication is going on TODO
  if (oscillator_gear0 <= g_c_gearbox_switch && g_c_gearbox_switch <= oscillator_gear8) {
    SystemInit_chip(g_c_gearbox_switch);
    g_c_gearbox_switch = GEAR_NO_CHANGE;
  }
  // ---
  ++*subtask; // dummy instruction to prevent compiler warning
  return 1; // always release this state, otherwise too complicated
}
