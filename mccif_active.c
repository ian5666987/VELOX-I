// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "Velox_checksum.h"

#include "mccif_xdata.h"

// --- MCCIF_ID_SET

short Mccif_formatSet(char system, short offset, short length, void* value, char* message) {
  unsigned short check;
  memcpy(message, mccif_xdatMarker, sizeof mccif_xdatMarker);
  message[4] = MCCIF_ID_SET;
  message[5] = system;
  message[6] = offset >> 8; // address hi
  message[7] = offset; // address lo
  message[8] = length; // length lo
  memcpy(&message[MCCIF_DATA_OFFSET], value, length);
  check = Checksum_publish(Checksum_update(message, MCCIF_DATA_OFFSET + length, 0));
  memcpy(&message[MCCIF_DATA_OFFSET + length], &check, 2);
  return MCCIF_DATA_OFFSET + length + 2;
}

// --- MCCIF_ID_REQUEST

short Mccif_formatRequest(char system, short offset, short length, char* message) {
  unsigned short check;
  memcpy(message, mccif_xdatMarker, sizeof mccif_xdatMarker);
  message[4] = MCCIF_ID_REQUEST;
  message[5] = mccif_localSystem;
  message[6] = 0;
  message[7] = 0;
  message[8] = 5;
  message[MCCIF_DATA_OFFSET + 0] = system; // receiver id
  message[MCCIF_DATA_OFFSET + 1] = offset >> 8; // address hi
  message[MCCIF_DATA_OFFSET + 2] = offset; // address lo
  message[MCCIF_DATA_OFFSET + 3] = length; // length lo
  check = Checksum_publish(Checksum_update(message, MCCIF_DATA_OFFSET + 4, 0));
  memcpy(&message[MCCIF_DATA_OFFSET + 4], &check, 2);
  return MCCIF_DATA_OFFSET + 4 + 2;
}

