// code by ntu ael

#include <c8051F120.h>
#include <string.h>
#include <stdio.h>

#include "shared_general.h"
#include "shared_obdh.h"

#include "Velox_timertask.h"
#include "Velox_watchdog.h"
#include "Velox_macros.h"

#include "mccif_buffer.h"
#include "mccif_local.h"
#include "mccif_xdata.h"

#include "comm_generic.h"

#include "obdh_cqt.h"
#include "obdh_globalvariables.h"
#include "obdh_constant.h"
#include "obdh_recon.h"
#include "obdh_schedule.h"

extern Timertask g_task_mccif;

char debug_container[32];
volatile char debug_index;

void Debug_init() {
  Mccif_localInit(MCCIF_UART, OBDH_ADDRESS);
  Mccif_setBuffer(debug_container, sizeof debug_container);
  Timertask_setRepeated(&g_task_mccif, 173); // large enough // 329
  debug_index = -1; // -1 so that first print index is 0
}

void Debug_loopDownlink() {
  char index;
  if (Timertask_isReady(&g_task_mccif))
    if (Recon_isDebugPrintEnabled())
      for (index = 0; index < XDATSEG_NUMEL; ++index) { // loop instead of a while to ensure will quit
        ++debug_index;
        debug_index %= XDATSEG_NUMEL;
        if (!debug_index)
          Mccif_localSend(XDATA_OBDH_PAS_OFFSET, XDATA_OBDH_PAS_LENGTH, xdata_txbuffer); // frame must not be visible via radio hk
        if (OBDH_ADDRESS == xdatSeg[debug_index].system) {
          Mccif_localSend(xdatSeg[debug_index].offset, xdatSeg[debug_index].length, xdata_txbuffer);
          break;
        }
      }
}
