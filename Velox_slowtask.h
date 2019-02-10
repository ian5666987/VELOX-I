// code by ntu ael

#ifndef VELOX_SLOWTASK_H
#define VELOX_SLOWTASK_H

enum enum_slowtask_mode {
  slowtask_disabled = 0, // == 0 required for initialization with memset!
  slowtask_executeOnce, //
  slowtask_periodic, // periodic mode compensates latency, for instance: if one execution is 20 ms late, then the next execution will be 20 ms early (i.e. back on schedule)!
  slowtask_repeated // repeated mode is also periodic but not strict, if the task is late for execution, it will _NOT_ try to be quicker next time!
};

struct struct_slowtask {
  unsigned char from_clock;
  unsigned char delay;
  enum enum_slowtask_mode mode;
};

typedef struct struct_slowtask Slowtask;

void Slowtask_setDisabled(Slowtask* task);
void Slowtask_setExecuteOnce(Slowtask* task, unsigned char delay);
void Slowtask_setPeriodic(Slowtask* task, unsigned char delay);
void Slowtask_setRepeated(Slowtask* task, unsigned char delay);
void Slowtask_fromNow(Slowtask* task);
unsigned char Slowtask_sinceLast(Slowtask* task);
char Slowtask_isReady(Slowtask* task);

#endif
