// code by ntu ael

#include "mccif_stream.h"
#include "mccif_buffer.h"

char* mccif_buffer = 0;
char mccif_buffer_index = 0;
short mccif_buffer_count = 0;
short mccif_buffer_maxlength;
short mccif_buffer_flush;

void Mccif_setBuffer(void* buffer, short maxlength) {
  mccif_buffer = (char*) buffer;
  mccif_buffer_index = 0;
  mccif_buffer_count = 0;
  mccif_buffer_maxlength = maxlength;
  mccif_buffer_flush = 0;
  Mccif_appendChars(0, 0, 0);
}

void Mccif_flush() {
  if (mccif_buffer && mccif_buffer_count) {
    Mccif_downlinkStream(mccif_buffer_index, mccif_buffer_flush, mccif_buffer_count, mccif_buffer);
    mccif_buffer_count = 0;
    ++mccif_buffer_flush;
  }
}

void Mccif_appendChar(char index, char value) {
  if (mccif_buffer) {
    if (mccif_buffer_index != index && mccif_buffer_count)
      Mccif_flush();
    // ---
    mccif_buffer_index = index;
    mccif_buffer[mccif_buffer_count] = value;
    ++mccif_buffer_count;
    if (mccif_buffer_count == mccif_buffer_maxlength)
      Mccif_flush();
  }
}

void Mccif_appendChars(char index, void* ptr, short len) {
  short c0;
  char* msg = (char*) ptr;
  for (c0 = 0; c0 < len; ++c0)
    Mccif_appendChar(index, msg[c0]);
}

