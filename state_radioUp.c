// code by ntu ael

#include <string.h>

#include "Velox_macros.h"

#include "mccif_local.h"
#include "mccif_spy.h"
#include "mccif_xdata.h"

#include "comm_imc.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"
#include "obdh_trxuv.h"

#include "state_util.h"

void StateUplink_prepareAck(char* message) {
  // if g_c_sendAck_flag == 1 an unprocessed ack might be overwritten, but that's ok
  memset(g_sendAck.arg, 0, sizeof g_sendAck.arg); // pad with zeros
  memcpy(&g_sendAck.xdatSeg, &message[MCCIF_SYSTEM_OFFSET], 4 + Mccif_getArgLength(message));
}

// indicate to gnd station that set instruction has been received and reached target
void StateUplink_releaseAck() {
  ++g_sendAck.uplinkId;
  g_sendAck.clock = Timertask_getClock_Override();
  g_c_sendAck_flag = 1;
}

// function returns true if message constitutes a valid set instruction for either OBDH itself, or PWRS or ADCS.
// in case the set instruction targets OBDH, the instruction has immediate effect
// if the set instruction is destined for PWRS or ADCS, the instruction is forwarded via SMBUS
// in the latter case, a StateUtil_peek handler is recommended to cleanly receive the confirmation
char Mccif_localHandle_Override(char* message) {
  char updated = 0;
  char system;
  if (message[4] == MCCIF_ID_SET) { // rather than MCCIF_ID_REQUEST
    system = message[MCCIF_SYSTEM_OFFSET];
    if (system == mccif_localSystem) {
      updated = Mccif_localSet(message);
      if (updated) {
        StateUplink_prepareAck(message);
        StateUplink_releaseAck(); // indicate to gnd station that uplink command has been received and reached target
      }
    } else if (Subsystem_isAvailable(system)) {
      StateUplink_prepareAck(message);
      StateUtil_pokeSetXdata(system, //
          extractFromBuffer(short, message + MCCIF_OFFSET_OFFSET), // MCCIF_SYSTEM_OFFSET + 1
          extractFromBuffer(unsigned char, message + MCCIF_LENGTH_OFFSET), // MCCIF_SYSTEM_OFFSET + 3
          message + MCCIF_DATA_OFFSET); // uses i2c_txbuffer
      updated = 1;
    }
  }
  return updated;
}

// ---

#define   TRXUVHK_PREPARE_REPLY_MS  8

char Auth_isPermitted_Override(void* buffer);
void Auth_localNotify_Override();

char State_radioUp(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  char upNumel; // just dummy variable
  char hasFrame; // used both to indicate how many frames are in rx buffer, but also if getFrame operation was successful
  short length;
  if (Trxuv_isAvailable()) {
    switch (*subtask) {
    case 0:
      Comm_rxQueueSizePut(); // request queue size of rx buffer
      Timertask_setExecuteOnce(&g_stateTask, TRXUVHK_PREPARE_REPLY_MS); // delay for a brief moment to allow ISISCOMM to prepare reply
      ++*subtask;
      break;
    case 1:
      if (Timertask_isReady(&g_stateTask)) // when delay has elapsed
        if (Comm_rxQueueSizeGet(&hasFrame)) { // get queue size of rx buffer
          if (hasFrame) { // numel != 0 -> buffer holds rx_frame
          	isRadioUp = 1;//ian 20140310: set this as true when we get uplink from the ground station!
            Comm_rxFramePut(); // announce to get the rxFrame from the buffer
            Timertask_setExecuteOnce(&g_stateTask, TRXUVHK_PREPARE_REPLY_MS + 8); // give extra time to copy frame data
            ++*subtask;
          } else
            release = 1; // uplink frame buffer empty
        } else
          // link to ISISCOMM had a problem, need to come back and retry (in this case the comm.failCount will increase, so far does not happen at all!)
          release = 1; // release due to smbus layer fail
      break;
    case 2:
      if (Timertask_isReady(&g_stateTask)) { // when delay has elapsed
        memset(xdata_txbuffer + 5, -1, 16); // clear rx buffer, this helps to indicate if appended bytes have been lost
        hasFrame = Comm_rxFrameGet(xdata_txbuffer + 5, &upNumel); // get rxFrame content
        Comm_rxFrameDrop(); // ALWAYS command ISISCOMM to drop rxFrame from buffer, DO NOT PUT THIS INSIDE IF, in case some insurmountable reason, otherwise stuck forever
        if (hasFrame) {
          memcpy(xdata_txbuffer, mccif_xdatMarker, sizeof mccif_xdatMarker); // XDAT prefix
          xdata_txbuffer[4] = MCCIF_ID_SET; // last byte of the 5 byte header
          if (Auth_isPermitted_Override(xdata_txbuffer)) {
            Runtime_fromNow(&g_trxuv.runtime);
            hasFrame = Mccif_localHandle_Override(xdata_txbuffer);
            if (hasFrame)
              if (g_sendAck.xdatSeg.system == mccif_localSystem) { // if the set instruction was intended for OBDH, the state can terminate right away
                Auth_localNotify_Override();
                release = 1;
              } else
                ++*subtask; // don't release yet, still needs confirmation from subsystem
            else
              release = 1; // message is corrupt and Mccif_localHandle_Override fails
          } else
            release = 1; // don't have permission to execute command
          // at this point, either a release=1, or ++*subtask has been given
        } else
          release = 1; // smbus layer fail, or data format error
      }
      break;
    case 3:
      if (StateUtil_peek(&length, &release) == reply_success) {
        Auth_localNotify_Override(); // a message to other subsystems also extends gnd pass clock
        StateUplink_releaseAck(); // indicate to gnd station that uplink command has been received and reached target
        release = 1;
      }
      break;
    }
  } else
    release = 1; // comm hardware not powered up
  return release;
}
