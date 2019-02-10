// code by ntu ael

#include <c8051F120.h>
#include <string.h>
#include <stdio.h>

#include "Velox_delay.h"
#include "Velox_macros.h"
#include "Velox_slowtask.h"
#include "Velox_state.h"
#include "Velox_uptime.h" //added to control beacon before the first hour pass

#include "mccif_spy.h"
#include "mccif_xdata.h"

#include "comm_generic.h"
#include "comm_itc.h"
#include "comm_itc_cw.h"

#include "shared_general.h"
#include "shared_obdh.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"
#include "obdh_trxuv.h"

#include "hash_blob.h"
#include "hash_util.h"

#include "state_downlink.h"
#include "state_util.h"

// the time delay to send 235 bytes via Ax25 (with 330kHz effective I2C clock rate - obtained from observing the oscilloscope -
// and 12 I2C clock cycle from one byte to the next one) is around 235 x 12/330,000 = 8.55ms + 5ms to wait for ISIS reply = 13.55ms.
#define DOWNLINK_MAX_DURATION_MS   20  // in milli seconds
#if DOWNLINK_MAX_DURATION_MS < 14 // > 13.55ms is sufficient
#error "out of sync: DOWNLINK_MAX_DURATION_MS"
#endif
#define   DOWNLINK_DROP_HEADER        5  // the old protocol includes the 5-byte header XDAT 0x00, the new downlink format has these 5-bytes removed
#define   DOWNLINK_ISSUED_AX25_MASK   0x01
#define   DOWNLINK_ISSUED_CW_MASK     0x02

#define   DOWNLINK_INACTIVE_TIME      7200 //beacon must be inactive during LEOP for this time span (in seconds). 20140520 - changed from 2700 to 7200 after VELOX-I meeting
// ---

volatile char downlink_issued; // flag inside this file to indicate if any logic causes uplink

// function is called when downlink is intended, if success
void private_downlink(void* buffer) {
  short length;
  length = Mccif_getFrameLength(buffer);
  if (Trxuv_isAvailable()) // don't remove! //This may need to distinguish between local PC and real ground station
    Comm_txAx25Put(((char*) buffer) + DOWNLINK_DROP_HEADER, length - DOWNLINK_DROP_HEADER); // single instance of call to Comm_txAx25Put in code
  // flag is set independent of comm hardware status:
  // * during mission comm is always on
  // * during development debug print should advance
  downlink_issued |= DOWNLINK_ISSUED_AX25_MASK;
  Recon_debugPrint(buffer, length);
}

void private_advanceGetFrameIndex() {
  ++g_getFrame.frameIndex.index;
  g_getFrame.frameIndex.index %= XDATSEG_NUMEL;
  if (0 == g_getFrame.frameIndex.index) { // into next page ?
    g_getFrame.frameIndex.page += g_getFrame.stride;
    g_getFrame.frameIndex.page %= g_hashFrame.pages; // here we go again
    --g_getFrame.numPages;
  }
}

void private_downlinkNextGetFrame() { //This function seems to be a bit unstable...
  char index;
  char status;
  for (index = 0; index < XDATSEG_NUMEL; ++index) {
    if ((g_getFrame.mask & (1L << g_getFrame.frameIndex.index)) != 0L) {
      status = Hash_getFrame(&g_getFrame.frameIndex, xdata_txbuffer, sizeof xdata_txbuffer);
      private_downlink(xdata_txbuffer);
      //private_advanceGetFrameIndex(); TOSHOW very dangerous duplicate!!
    }
    private_advanceGetFrameIndex();
    if (!g_getFrame.numPages)
      break;
  }
}

void private_downlinkNextPinFrame() {
  char index;
  char status;
  for (index = 0; index < XDATSEG_NUMEL; ++index) {
    if ((g_pinFrame.mask & (1L << g_pinFrame.frameIndex.index)) != 0L) {
      status = Hash_getFrame(&g_pinFrame.frameIndex, xdata_txbuffer, sizeof xdata_txbuffer);
      private_downlink(xdata_txbuffer);
      if (status)
        printf("pin ok ");
      else
        printf("pin wrong ");
      ++g_pinFrame.frameIndex.index;
    }
    ++g_pinFrame.frameIndex.index;
    if (g_pinFrame.frameIndex.index == XDATSEG_NUMEL)
      break;
  }
}

void private_downlinkBlob(short count) {
  char success;
  success = Hash_getBlob(g_getBlob.index, g_getBlob.type, count, xdata_txbuffer, sizeof xdata_txbuffer);
  if (success) // gnd station cannot process corrupt frame, but instead has to monitor the parameter SD_read_error
    private_downlink(xdata_txbuffer);
}

#define BEACON_HEADER_LENGTH 7
void private_downlinkBeacon() {
  if (Trxuv_isAvailable()) { // don't remove!
    xdata_txbuffer[0] = 'S';
    xdata_txbuffer[1] = 'V';
    xdata_txbuffer[2] = 'E';
    xdata_txbuffer[3] = 'L';
    xdata_txbuffer[4] = 'O';
    xdata_txbuffer[5] = 'X';
    xdata_txbuffer[6] = 'I';
    memcpy(&xdata_txbuffer[BEACON_HEADER_LENGTH], g_c_beacon, sizeof g_c_beacon);
    Comm_setTxCwCharRate(g_trxuv.rateCW);
    Comm_txCwPut(xdata_txbuffer, BEACON_HEADER_LENGTH + sizeof g_c_beacon); // function call already includes a delay for smbus tx completion
  }
  downlink_issued |= DOWNLINK_ISSUED_CW_MASK; // according to tests, the beacon has effect on the tx_available
}

extern Statetask g_statetask_downlink;

#define   DOWNLINK_BUFFER_SIZE           5 // number of different return values {0, 1, 2, 3, 4} from isis comm board when asking how many tx frames are available for buffering
#define   DOWNLINK_COMPLETED_SEC         4 // upper bound of seconds after which we assume a single frame has been removed from tx buffer
#define   DOWNLINK_IDLE_ACTIVE_OFF_SEC   2 // number of seconds after which tx idle active is disabled
// in idle active mode at 1200 bps, the downlink of 110 frames of 231 bytes each takes 191.97 sec
// => 1 frame takes 1.7452 sec, data rate is 132.36 B/sec

char State_downlink(char* subtask) {
  char release = 0;
  char index; // has to be signed!
  unsigned char length;
  if (DOWNLINK_COMPLETED_SEC < Slowtask_sinceLast(&g_slow_beacon)) // assume after a few seconds of no downlink command -> can assume that one slot in buffer is vacant
    g_statetask_downlink.task.delay = STATE_DONWLINK_DELAY_MIN; // this ensures that uplinks are acknowledged swiftly, and any new downlink are processed
  // ---
  if (Trxuv_isAvailable() || Recon_isDebugPrintEnabled()) {
    switch (*subtask) {
    case 0:
      downlink_issued = 0; // will be overridden in case of downlink TODO bad style
      // --- Acknowledgment = 1
      if (!downlink_issued)
        if (g_c_sendAck_flag) { // send acknowledgement for uplink
        	if (isRadioUp){ //ian 20140310: only sends the acknowledgment downlink for uplink if the uplink is from the radio
						length = Mccif_formatContent(XDATA_OBDH_ACK_OFFSET, XDATA_OBDH_ACK_LENGTH, xdata_txbuffer);
						private_downlink(xdata_txbuffer);
        	}
          g_c_sendAck_flag = 0;
        }
      // --- Recovery (low) = 2
      if (!downlink_issued) //recovery message has the highest priority apart from acknowledgement
        for (index = 0; index < GETBLOB_LO_NUMEL; ++index)
          if (g_getBlob.lo[index] != DOWNLINK_IGNORE_LO) {
            private_downlinkBlob(g_getBlob.lo[index]); // even if fails, do not retry or attempt next
            g_getBlob.lo[index] = DOWNLINK_IGNORE_LO;
            break;
          }
      // --- Recovery (hi) = 3
      if (!downlink_issued)
        for (index = 0; index < GETBLOB_HI_NUMEL; ++index)
          if (g_getBlob.hi[index] != DOWNLINK_IGNORE_HI) {
            private_downlinkBlob(g_getBlob.hi[index]); // even if fails, do not retry or attempt next
            g_getBlob.hi[index] = DOWNLINK_IGNORE_HI;
            break;
          }
      // --- Blob = 4
      if (!downlink_issued)
        if (g_getBlob.head < g_getBlob.tail) { //Blob data has the second highest priority after recovery
          private_downlinkBlob(g_getBlob.head); // even if fails, do not retry or attempt next
          ++g_getBlob.head;
        }
      // --- WOD = 5
      if (!downlink_issued) //WOD data has the third highest priority
        if (g_getFrame.mask != 0L && 0 < g_getFrame.numPages)
          private_downlinkNextGetFrame();
      // --- Pin = 6
      if (!downlink_issued) //this will only send certain type of AX25 message
        if (g_pinFrame.mask != 0L && g_pinFrame.frameIndex.index < XDATSEG_NUMEL)
          private_downlinkNextPinFrame();
      // --- To delay the groundpass termination
      if (downlink_issued && g_slow_gndPass.mode == slowtask_executeOnce) //if it has downlink issued here while in the groundpass
      	Slowtask_fromNow(&g_slow_gndPass); // postpone groundpass task to be terminated
      // --- Real_time HK = 7
      if (!downlink_issued)
        if (g_rtHk.hasNext) { //should have the least priority (apart from unused item)!
          g_rtHk.hasNext = 0; // clear flag
          private_downlink(txframeRelay);
        }
      // --- CW beacon = 8
      if (uptime.total >= DOWNLINK_INACTIVE_TIME && g_slow_gndPass.mode == slowtask_disabled) { //don't send any CW beacon before this time elapses
        if (downlink_issued) // was ax25 frame sent
          Slowtask_fromNow(&g_slow_beacon); // prevents beacon to be sent right after
        else if (Slowtask_isReady(&g_slow_beacon)) // check if beacon downlink is encouraged
          private_downlinkBeacon();
      }
      // ---
      if (downlink_issued) { // if either ax25 or beacon was sent
        Timertask_setExecuteOnce(&g_stateTask, DOWNLINK_MAX_DURATION_MS);
        ++*subtask;
      } else { // in case no downlink
        if ((DOWNLINK_IDLE_ACTIVE_OFF_SEC < Slowtask_sinceLast(&g_slow_beacon) && g_slow_beacon.mode == slowtask_repeated)
        		|| g_slow_beacon.mode == slowtask_disabled)// timeout period of no downlink
          if (Trxuv_isAvailable() && Trxuv_isIdleActive())
            Comm_setTxIdleState(0); // this required for safety. do not remove, since otherwise COMM drains too much power etc.
        release = 1;
      }
      break;
    case 1: // when either beacon or ax25 frame has just been commissioned
      if (Timertask_isReady(&g_stateTask)) { // when delay has elapsed
        if (Trxuv_isAvailable()) { // comm still on
          if (Comm_txAx25AndCwGet(&index)) { // get tx available / send status
            if (0 <= index && index < DOWNLINK_BUFFER_SIZE) // check return value
              g_statetask_downlink.task.delay = g_trxuv.delayMap[index] / Trxuv_getTxRateFactor();
            else {
              g_statetask_downlink.task.delay = g_trxuv.delayMap[2] / Trxuv_getTxRateFactor(); // index == 0
              printf("!IDX %bd ", index);
            }
          }
          if (downlink_issued & DOWNLINK_ISSUED_AX25_MASK) // was ax25 frame issued
            if (!Trxuv_isIdleActive() && g_trxuv.autoIdleActive)
              Comm_setTxIdleState(1);
        }
        release = 1; // after delay, release state in any case
      }
      break;
    }
  }
  return release;
}
