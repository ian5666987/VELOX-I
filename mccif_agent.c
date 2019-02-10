// code by ntu ael

#include <string.h>

#include "C8051_SMBus.h"

#include "Velox_checksum.h"
#include "Velox_delay.h"
#include "Velox_macros.h"

#include "mccif_xdata.h"

char* mccif_agent_tx;

// buffer should hold at least 240 bytes
void Mccif_agentInit(void* buffer) {
  mccif_agent_tx = (char*) buffer;
}

// parameter list to function might not be final, since can easily supply message if needed
void Mccif_changeAction();

// --- MCCIF_ID_REQUEST

short Mccif_requestContent(char* message, char* buffer) { // parameter name required for macro
  char success; // name required for macro
  short offset, length;
  success = memcmp(message, mccif_xdatMarker, sizeof mccif_xdatMarker) == 0;
  success &= message[4] == MCCIF_ID_REQUEST;
  // message[5] has issuing system
  success &= extractFromBuffer(short, message + MCCIF_OFFSET_OFFSET) == 0;
  success &= extractFromBuffer(unsigned char, message + MCCIF_LENGTH_OFFSET) == 5;
  success &= message[MCCIF_DATA_OFFSET] == mccif_localSystem;
  if (success) {
    success &= Checksum_isValid(message, MCCIF_DATA_OFFSET + 4 + 2);
    if (success) {
      offset = extractFromBuffer(short, message + MCCIF_DATA_OFFSET + 1);
      length = extractFromBuffer(unsigned char, message + MCCIF_DATA_OFFSET + 3);
      success &= offset && 0 < length && length < 245; // upper bound is only approximated to 255 - MCCIF_DATA_OFFSET - 2 + 1
      if (success)
        length = Mccif_formatContent(offset, length, buffer);
    }
  }
  return success ? length : 0;
}

char Mccif_localHandle_Override(char* message) {
  char updated = 0;
  short length;
  updated |= Mccif_localSet(message);
  if (updated) {
    Mccif_changeAction(); // actuators need to be changed immediately, otherwise changes do not have effect at all!
  }
  length = Mccif_requestContent(message, mccif_agent_tx);
  if (length) {
    SMBus_setTxBuffer(mccif_agent_tx, length);
    SMBus_start(SMB_writeTo, message[5]);
    updated = 1;
  }
  return updated;
}

void Mccif_setCompleted_Override(short offset) {
  short length;
  // pwrs and adcs are expected to be instructed by obdh
  // obdh expects the following reply as confirmation within ~80 ms, i.e. plenty of time
  length = Mccif_formatContent(offset, 0, mccif_agent_tx); // empty content with offset as identifier
  if (length) {
    SMBus_setTxBuffer(mccif_agent_tx, length); // send message via SMBus
    SMBus_start(SMB_writeTo, 0); // universal send
    Delay_blockMs(2); // TODO potentially use flag from SMBus to quit waiting!
  }
}
