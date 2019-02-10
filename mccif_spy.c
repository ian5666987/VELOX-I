// code by ntu ael

#include <string.h>

#include "velox_macros.h"
#include "velox_checksum.h"

#include "mccif_xdata.h"

short Mccif_getArgLength(void* buffer) {
  char* message = (char*) buffer; // name required for macro
  return extractFromBuffer(unsigned char, message + MCCIF_LENGTH_OFFSET);
}

short Mccif_getFrameLength(void* buffer) {
  return MCCIF_DATA_OFFSET + Mccif_getArgLength(buffer) + 2; // 2 = length of checksum
}

unsigned short Mccif_getBlobIndex(void* buffer) {
  char* message = (char*) buffer;
  return extractFromBuffer(unsigned short, message + MCCIF_DATA_OFFSET);
}

char Mccif_getBlobType(void* buffer) {
  char* message = (char*) buffer;
  return extractFromBuffer(char, message + MCCIF_DATA_OFFSET + 2);
}

unsigned short Mccif_getBlobCount(void* buffer) {
  char* message = (char*) buffer;
  return extractFromBuffer(short, message + MCCIF_DATA_OFFSET + 3);
}

// also empty content (length==0) is considered valid
// frames with empty content (length==0) are sent as a confirmation reply for set commands
char Mccif_isValidContent(void* buffer, short max_length) {
  char success = 0;
  char* message = (char*) buffer;
  short length;
  if (MCCIF_DATA_OFFSET + 2 <= max_length) { // message must at least hold header and crc
    success = memcmp(message, mccif_xdatMarker, sizeof mccif_xdatMarker) == 0;
    length = extractFromBuffer(unsigned char, message + MCCIF_LENGTH_OFFSET);
    success &= 0 <= length && MCCIF_DATA_OFFSET + length + 2 <= max_length; // converts to content length for comparison
    success &= Checksum_isValid(message, MCCIF_DATA_OFFSET + length + 2);
  }
  return success;
}

