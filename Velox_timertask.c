// code by ntu ael

#include "Velox_timertask.h"

 // Timertask_setDisabled is implemented so that it suits the situation as often as possible
void Timertask_setDisabled(Timertask* task) {
  task->mode = timertask_disabled;
}

void Timertask_setExecuteOnce(Timertask* task, unsigned short delay) {
  task->from_clock = Timertask_getClock_Override();
  task->delay = delay;
  task->mode = timertask_executeOnce;
}

void Timertask_setPeriodic(Timertask* task, unsigned short delay) {
  task->from_clock = Timertask_getClock_Override();
  task->delay = delay;
  task->mode = timertask_periodic;
}

void Timertask_setRepeated(Timertask* task, unsigned short delay) {
  task->from_clock = Timertask_getClock_Override();
  task->delay = delay;
  task->mode = timertask_repeated;
}

// Timertask_fromNow is typically used to postpone the next task execution,
// since subsequently the full delay is required to elapse
// Examples: watch-dog mechanisms, prevent state buildups, ...
void Timertask_fromNow(Timertask* task) {
  task->from_clock = Timertask_getClock_Override();
}

char Timertask_isReady(Timertask* task) {
  unsigned short delta;
  unsigned short timertask_clock = Timertask_getClock_Override();
  if (task->mode == timertask_disabled) {
    task->from_clock = timertask_clock;
  } else {
    delta = timertask_clock - task->from_clock;
    if (task->delay <= delta) {
      switch (task->mode) {
      case timertask_executeOnce:
        task->from_clock = timertask_clock;
        task->mode = timertask_disabled;
        break;
      case timertask_periodic:
        task->from_clock += task->delay; // equation to compensate lag
        break;
      case timertask_repeated:
        task->from_clock = timertask_clock; // ignore any lag; pretend you are on time, and next execution is in 'delay' ms
        break;
      }
      return 1;
    }
  }
  return 0;
}
