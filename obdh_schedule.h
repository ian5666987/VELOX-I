// code by ntu ael

#ifndef OBDH_SCHEDULE_H
#define OBDH_SCHEDULE_H

#include "mccif_tools.h"
#include "mccif_xdata.h"

#define SCHEDULE_SIZE_LIMIT 48

struct SchedulerStruct { // 4 + 1 + 2 + 1 + 10 = 18
  unsigned long time;
  XdataSegment xdatSeg;
  char arg[MCCIF_COMMAND_MAXLEN];
};

typedef struct SchedulerStruct ScheduleItem;

struct struct_scheduleAdd {
  char id0;
  ScheduleItem item; //to add schedule by calling this outside of schedule library
  char id1;
};

extern struct struct_scheduleAdd g_scheduleAdd;

void Schedule_init();
void Schedule_remove(unsigned char beg, unsigned char end);
char Schedule_handleAdd();
char Schedule_hasNext();

#endif
