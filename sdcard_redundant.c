// code by ntu ael

#include "sdcard_generic.h"
#include "sdcard_redundant.h"

char (*SDred_check_func)(void*, short);
char* SDred_readback;

extern unsigned short sd_card_store_failures;

void SDred_init(char (*check_func)(void*, short), void* buffer) {
  SDred_check_func = check_func;
  SDred_readback = (char*) buffer;
}

// returns 1 for successful reading, then buffer contains sd card page up to length bytes
// 0 for unsuccessful, then buffer content up to length bytes is undefined
char SDred_read(unsigned long start_block, void* buffer, short length, char copies) {
  char index;
  char success;
  for (index = 0; index < copies; ++index) {
    success = SD_readSingleBlock(start_block + index, buffer, length);
    ++g_sdcard.read.count;
    if (success && SDred_check_func(buffer, length))
      return 1;
    else {
      ++g_sdcard.read.failures;
      g_sdcard.read.lastFailAddress = start_block + index;
      // ---
      success = SD_readSingleBlock(start_block + index, buffer, length); // try again once
      ++g_sdcard.read.count;
      if (success && SDred_check_func(buffer, length))
        return 1;
    }
  }
  return 0;
}

char SDred_write(unsigned long start_block, void* buffer, short length, char copies) {
  char index;
  char success;
  char numel = 0;
  for (index = 0; index < copies; ++index) {
    success = SD_writeSingleBlock(start_block + index, buffer, length);
    ++g_sdcard.write.count;
    if (success) {
      ++numel;
      // check right away by read-back data once, and if necessary rewrite once, then proceed to next block
      if (!SDred_read(start_block + index, SDred_readback, length, copies)) { // if read-back is not successful
        ++g_sdcard.write.failures; // ... increment failure count
        g_sdcard.write.lastFailAddress = start_block + index;
        success = SD_writeSingleBlock(start_block + index, buffer, length); // give it one more try
        numel += success;
      }
    }
    // ---
    if (!success) {
      ++g_sdcard.write.failures;
      g_sdcard.write.lastFailAddress = start_block + index;
    }
  }
  if (!numel) //If there is no successful storing at all, increases this.
    ++sd_card_store_failures;
  return numel; // Return the number of time of successful writing
}
