// code by ntu ael

#ifndef OBDH_DEBUG_H
#define OBDH_DEBUG_H

struct struct_gndCommand { // 2 + 2 + 2 + 4 = 10 (size of one uplink)
  short id;
  short parameter;
//  short helper;
//  long address;
};

extern struct struct_gndCommand g_gndCommand;

void Debug_init();
void Debug_loopDownlink();

#endif
