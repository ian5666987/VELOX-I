// code by ntu ael

#include <c8051f120.h>
#include <string.h>
//#include <stdio.h>

#include "c8051_SMBus.h"

#include "Velox_slowtask.h"
#include "Velox_uart.h"

#include "mccif_active.h"
#include "mccif_xdata.h"

#include "shared_general.h"
#include "shared_pwrs.h"

#include "comm_generic.h"

#include "hash_frame.h"
#include "hash_util.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"

#include "state_util.h"

// state "xdatSegGet" collects all packets for
// 1) periodically storing on sd card (private_xdatSegSdStorage)
// 2) downlink as real time housekeeping data (private_xdatSegPropagate)
// 3) debug monitoring (Recon_debugPrint)
// ---

// 1) periodic storing on sd card
void private_xdatSegSdStorage(char index, void* buffer, short length) {
  if (index == 0 && g_putFrame.putIterator == putIterator_active) { // can cancel SD card storage iteration?
    g_putFrame.putIterator = putIterator_inactive; // stop log sequence
    ++g_putFrame.frameIndex.page; // prepare next page address
    g_putFrame.frameIndex.page %= g_hashFrame.pages; // display of page shall not exceed total number of pages
  }
  // ---
  if (Slowtask_isReady(&g_putFrame.task)) // opportunity to change from SDcard inactive to fused, otherwise deliberately let opportunity slip
    if (g_putFrame.putIterator == putIterator_inactive) // change from inactive ...
      g_putFrame.putIterator = putIterator_fused; // ... to fused
  // ---
  if (index == 0 && g_putFrame.putIterator == putIterator_fused) // if new cycle has just started, and SD card is marked for storage
    g_putFrame.putIterator = putIterator_active; // iterator is marked for active logging
  // ---
  if (SDCard_isAvailable() && g_putFrame.putIterator == putIterator_active) { // only store under these conditions
    g_putFrame.frameIndex.index = index; // store index for next comparison
    if ((g_putFrame.mask & (1L << index)) != 0L)
      Hash_putFrame(&g_putFrame.frameIndex, buffer, length);
  }
}

// 2) downlink as real time housekeeping data
void private_xdatSegPropagate(char index, void* buffer, short length) {
  unsigned long bitmask;
  if (!g_rtHk.hasNext) { // frame buffer is empty
    bitmask = 1L << index;
    if ((g_rtHk.mask_now & bitmask) != 0L) { // ground station has selected frame for downlink
      g_rtHk.mask_now ^= bitmask;
      memcpy(txframeRelay, buffer, length);
      g_rtHk.hasNext = 1;
    }
  }
}

volatile char xdatSeg_cycle = -1; // so that first index is 0

// this task helps to monitor the parameters of all subsystems during development
// the monitoring happens at a high frequency
// the sd card storage happens at a low frequency
char State_xdatSegGet(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  char address;
  short length;
  unsigned long bitmask;
  switch (*subtask) {
  case 0: // sending request
    ++xdatSeg_cycle;
    xdatSeg_cycle %= XDATSEG_NUMEL;
    address = xdatSeg[xdatSeg_cycle].system;
    if (address == mccif_localSystem) {
      length = Mccif_formatContent(xdatSeg[xdatSeg_cycle].offset, xdatSeg[xdatSeg_cycle].length, xdata_txbuffer);
      // ---
      private_xdatSegSdStorage(xdatSeg_cycle, xdata_txbuffer, length); // sd card storage
      private_xdatSegPropagate(xdatSeg_cycle, xdata_txbuffer, length); // prepare for downlink
      Recon_debugPrint(xdata_txbuffer, length); // debug monitoring
      release = 1;
    } else { // xdata segment is on other chip
      if (Subsystem_isAvailable(address)) { // only if the subsystem is ON and "available" we will get the data segment from them
        StateUtil_pokeRequestXdata( //
            xdatSeg[xdatSeg_cycle].system, //
            xdatSeg[xdatSeg_cycle].offset, //
            xdatSeg[xdatSeg_cycle].length);
        ++*subtask;
      } else {
        bitmask = 1L << xdatSeg_cycle;
        if ((g_rtHk.mask_now & bitmask) != 0L)
          g_rtHk.mask_now ^= bitmask; // since system is down, rtHK for this system can not be sent, and is therefore considered "sent"
        release = 1; // immediately fail
      }
    }
    break;
  case 1:
    if (StateUtil_peek(&length, &release) == reply_success) {
      private_xdatSegSdStorage(xdatSeg_cycle, i2c_rxbuffer, length); // sd card storage
      private_xdatSegPropagate(xdatSeg_cycle, i2c_rxbuffer, length); // prepare for downlink
      Recon_debugPrint(i2c_rxbuffer, length); // debug monitoring
      release = 1;
    }
    break;
  }
  return release;
}
