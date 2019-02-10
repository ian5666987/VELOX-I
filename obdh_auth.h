// code by ntu ael

#ifndef OBDH_AUTH_H
#define OBDH_AUTH_H

#include "Velox_slowtask.h"

enum enum_auth {
  auth_normal = 0, //
  auth_request,
  auth_inject, //
  auth_expire, //
};

union union_keypass {
  unsigned long value;
  unsigned short array[2];
};

struct struct_auth { // 1 + 4 + 4 // when not in gnd pass, the gnd station can only write to auth_control and auth_unlock
  enum enum_auth control;
  unsigned long unlock;
  // --- protected
  union union_keypass keycode;
};

extern struct struct_auth g_auth;

void Auth_init();

#endif
