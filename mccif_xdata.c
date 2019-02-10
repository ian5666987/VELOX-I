// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "Velox_checksum.h"
#include "Velox_uart.h"
#include "Velox_macros.h"

#include "mccif_xdata.h"

char code mccif_xdatMarker[4] = {'X', 'D', 'A', 'T'};

// --- MCCIF_ID_CONTENT

/** buffer needs to hold MCCIF_DATA_OFFSET + length + 2 bytes
 * function inserts memory image into buffer.
 * parameter length is valid for numbers only up to 255 bytes. */
short Mccif_formatContent(short offset, short length, char* message) {
  char xdata* ptr = 0;
  unsigned short check;
  memcpy(message, mccif_xdatMarker, sizeof mccif_xdatMarker);
  message[4] = MCCIF_ID_CONTENT;
  message[5] = mccif_localSystem;
  message[6] = offset >> 8; // address hi
  message[7] = offset; // address lo
  message[8] = length; // length lo
  memcpy(&message[MCCIF_DATA_OFFSET], ptr + offset, length);
  check = Checksum_publish(Checksum_update(message, MCCIF_DATA_OFFSET + length, 0));
  memcpy(&message[MCCIF_DATA_OFFSET + length], &check, 2);
  return MCCIF_DATA_OFFSET + length + 2;
}

// --- MCCIF_ID_SET

char Mccif_localSet(char* message) { // parameter name required for macro
  char success; // name required for macro
  short offset, length;
  char xdata* ptr = 0;
  success = memcmp(message, mccif_xdatMarker, sizeof mccif_xdatMarker) == 0;
  success &= message[4] == MCCIF_ID_SET; // set
  success &= message[MCCIF_SYSTEM_OFFSET] == mccif_localSystem;
  offset = extractFromBuffer(short, message + MCCIF_OFFSET_OFFSET);
  length = extractFromBuffer(unsigned char, message + MCCIF_LENGTH_OFFSET);
  success &= offset && 0 < length && length <= MCCIF_COMMAND_MAXLEN;
  if (success) {
    success &= Checksum_isValid(message, MCCIF_DATA_OFFSET + length + 2);
    if (success) {
      EA = 0;
      EA = 0;
      memcpy(ptr + offset, &message[MCCIF_DATA_OFFSET], length);
      EA = 1;
      Mccif_setCompleted_Override(offset);
    }
  }
  return success;
}

