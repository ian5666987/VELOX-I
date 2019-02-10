// code by ntu ael

#ifndef VELOX_STATE_H
#define VELOX_STATE_H

#include "Velox_timertask.h"

struct struct_statetask {
  Timertask task; // repeated timertask
  char fused; // when true means execute statetask as soon as possible
};

typedef struct struct_statetask Statetask;

struct struct_state { // 3 + 1 + 4 + 1 = 9
  Statetask* current; // pointer to current statetask, or 0 if no statetask is presently active (i.e. due to lack of readiness)
  char subtask; // helps to discriminate subtasks within current statetask. the only variable that is visible to statetask implementation
  unsigned short from_clock; // time when current state was first ready
  unsigned short timeout; // timeout in ms after which current statetask is forcefully released
  unsigned char failures; // number of times any state did not release before timeout
};

typedef struct struct_state State; // there is only once instance of State, but many instances of Statetask

void State_timeout_Override(State* state);

void State_starter(State* state, unsigned short value);
void State_initStatetask(Statetask* statetask, unsigned short delay);
void State_turing(State* state, Statetask* statetask, char (*state_func)(char*));

#endif
