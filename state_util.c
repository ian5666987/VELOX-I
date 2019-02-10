// code by ntu ael

#include <string.h>

#include "C8051_SMBus.h"
#include "C8051_SMBusSpy.h"

#include "Velox_macros.h"

#include "mccif_active.h"
#include "mccif_spy.h"
#include "mccif_tools.h"
#include "mccif_xdata.h"

#include "state_util.h"

XdataSegment stateLastout;
Timertask g_stateTask; // can be used by any state

char* stateutil_write;
char* stateutil_read;
short stateutil_readMax;

void StateUtil_init(void* i2c_write, void* i2c_read, short i2c_readMax) {
  stateutil_write = i2c_write;
  stateutil_read = i2c_read;
  stateutil_readMax = i2c_readMax;
}

void StateUtil_pokeRequestXdata(char system, short offset, short length) {
  short size;
  size = Mccif_formatRequest(system, offset, length, stateutil_write);
  SMBus_setTxBuffer(stateutil_write, size);
  SMBus_setRxBuffer(stateutil_read, stateutil_readMax);
  SMBus_start(SMB_writeTo, system);
  stateLastout.system = system;
  stateLastout.offset = offset;
  stateLastout.length = length;
  Timertask_setExecuteOnce(&g_stateTask, g_intercom.timeout);
}

void StateUtil_pokeSetXdata(char system, short offset, short length, void* value) {
  short size;
  size = Mccif_formatSet(system, offset, length, value, stateutil_write);
  SMBus_setTxBuffer(stateutil_write, size);
  SMBus_setGeneralReceiver(1);
  SMBus_start(SMB_writeTo, system);
  stateLastout.system = system;
  stateLastout.offset = offset;
  stateLastout.length = 0; // in the reply to poke only system and offset are encoded, while length is projected to 0
  Timertask_setExecuteOnce(&g_stateTask, g_intercom.timeout);
}

enum enum_replyResult StateUtil_peek(short* length, char* release) {
  char success;
  if (SMBus_isRxComplete(length)) {
    success = Mccif_isValidContent(stateutil_read, stateutil_readMax);
    success &= stateutil_read[4] == MCCIF_ID_CONTENT;
    success &= memcmp(&stateLastout.system, stateutil_read + MCCIF_SYSTEM_OFFSET, sizeof stateLastout) == 0;
    if (success) {
      SMBus_setGeneralReceiver(0);
      return reply_success;
    }
    ++g_intercom.invalids; // monitoring only
    return reply_invalid; // if reply is no match, can still continue until timeout
  } else if (Timertask_isReady(&g_stateTask)) {
    SMBus_setGeneralReceiver(0);
    g_intercom.lastFailedSystem = stateLastout.system; // monitoring only
    ++g_intercom.timeouts;
    *release |= 1;
    // ---
    return reply_timeout;
  }
  return reply_pending;
}
