// code by ntu ael

#ifndef STATE_UTIL_H
#define STATE_UTIL_H

#include "Velox_timertask.h"

enum enum_replyResult {
  reply_pending, // <- typically does not require action in main code
  reply_invalid, // <- typically does not require action in main code
  reply_success, //
  reply_timeout, // <- typical action is to release the state, rather than holding on
};

struct struct_intercom {
  short timeout; // ms
  short invalids;
  short timeouts;
  char lastFailedSystem;
};

extern struct struct_intercom g_intercom;
extern Timertask g_stateTask;

#define  StateUtil_copyReply(ptr, len)    memcpy(ptr, &i2c_rxbuffer[MCCIF_DATA_OFFSET], len)

void StateUtil_init(void* i2c_write, void* i2c_read, short i2c_read_max);
void StateUtil_pokeRequestXdata(char system, short offset, short length);
void StateUtil_pokeSetXdata(char system, short offset, short length, void* buffer);
enum enum_replyResult StateUtil_peek(short* length, char* release);

#endif
