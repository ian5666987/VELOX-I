// code by ntu ael

#ifndef SDCARD_REDUNDANT_H
#define SDCARD_REDUNDANT_H

struct struct_sdcard_track { // 7
  short count;
  char failures;
  unsigned long lastFailAddress;
};

struct struct_sdcard { // 1 + 7 + 7 = 15 bytes
  char isAvailable;
  struct struct_sdcard_track write;
  struct struct_sdcard_track read;
};

extern struct struct_sdcard g_sdcard;

void SDred_init(char (*check_func)(void*, short), void* buffer);
char SDred_read(unsigned long start_block, void* buffer, short length, char copies);
char SDred_write(unsigned long start_block, void* buffer, short length, char copies);

#endif
