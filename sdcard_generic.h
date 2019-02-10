// code by ntu ael

#ifndef SDCARD_GENERIC_H
#define SDCARD_GENERIC_H

struct struct_sdConfig_time {
  unsigned short init;        //Delkin: 1 | SANDisk: 7000
  unsigned short rnw;         //Delkin: 1 | SANDisk: 20
  unsigned short store;       //Delkin: 150 | SANDisk: 200
};

struct struct_sdConfig_retry {
  unsigned char cmd;          //Delkin: 5 | SANDisk: 5
  unsigned char cmd_reply;    //Delkin: 5 | SANDisk: 5
  unsigned char store_reply;  //Delkin: 100 | SANDisk: 100
};

struct struct_sdConfig {
  struct struct_sdConfig_time time;
  struct struct_sdConfig_retry retry;
};

typedef struct struct_sdConfig SdConfig;

extern SdConfig g_sdConfig;

void SD_initConfig();
char SD_init();
char SD_writeSingleBlock(unsigned long block, void* buffer, short length);
char SD_readSingleBlock(unsigned long block, void* buffer, short length);

#endif

