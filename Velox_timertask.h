// code by ntu ael

#ifndef VELOX_TIMERTASK_H
#define VELOX_TIMERTASK_H

enum enum_timertask_mode {
  timertask_disabled = 0, // == 0 required for initialization with memset!
  timertask_executeOnce, //
  timertask_periodic, // periodic mode compensates latency, for instance: if one execution is 20 ms late, then the next execution will be 20 ms early (i.e. back on schedule)!
  timertask_repeated // repeated mode is also periodic but not strict, if the task is late for execution, it will _NOT_ try to be quicker next time!
};

struct struct_timertask {
  unsigned short from_clock;
  unsigned short delay;
  enum enum_timertask_mode mode;
};

typedef struct struct_timertask Timertask;

unsigned short Timertask_getClock_Override();

void Timertask_setDisabled(Timertask* task);
void Timertask_setExecuteOnce(Timertask* task, unsigned short delay);
void Timertask_setPeriodic(Timertask* task, unsigned short delay);
void Timertask_setRepeated(Timertask* task, unsigned short delay);
void Timertask_fromNow(Timertask* task);
char Timertask_isReady(Timertask* task);

#endif
