// code by ntu ael

#include "Velox_slowtask.h"
#include "Velox_uptime.h"

// Slowtask_setDisabled is implemented so that it suits the situation as often as possible
void Slowtask_setDisabled(Slowtask* task) {
  task->mode = slowtask_disabled;
}

void Slowtask_setExecuteOnce(Slowtask* task, unsigned char delay) {
  task->from_clock = uptime.session;
  task->delay = delay;
  task->mode = slowtask_executeOnce;
}

void Slowtask_setPeriodic(Slowtask* task, unsigned char delay) {
  task->from_clock = uptime.session;
  task->delay = delay;
  task->mode = slowtask_periodic;
}

void Slowtask_setRepeated(Slowtask* task, unsigned char delay) {
  task->from_clock = uptime.session;
  task->delay = delay;
  task->mode = slowtask_repeated;
}

// Slowtask_fromNow is typically used to postpone the next task execution,
// since subsequently the full delay is required to elapse
// Examples: watch-dog mechanisms, prevent state buildups, ...
void Slowtask_fromNow(Slowtask* task) {
  task->from_clock = uptime.session;
}

// returns number of seconds since last execution
unsigned char Slowtask_sinceLast(Slowtask* task) {
  unsigned char slowtask_clock = uptime.session;
  return slowtask_clock - task->from_clock;
}

char Slowtask_isReady(Slowtask* task) {
  unsigned char delta;
  unsigned char slowtask_clock = uptime.session;
  if (task->mode == slowtask_disabled) {
    task->from_clock = slowtask_clock;
  } else {
    delta = slowtask_clock - task->from_clock;
    if (task->delay <= delta) {
      switch (task->mode) {
      case slowtask_executeOnce:
        task->from_clock = slowtask_clock;
        task->mode = slowtask_disabled;
        break;
      case slowtask_periodic:
        task->from_clock += task->delay; // equation to compensate lag
        break;
      case slowtask_repeated:
        task->from_clock = slowtask_clock; // ignore any lag; pretend you are on time, and next execution is in 'delay' ms
        break;
      }
      return 1;
    }
  }
  return 0;
}
