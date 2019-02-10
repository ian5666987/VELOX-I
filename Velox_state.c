// code by ntu ael

#include "Velox_state.h"

#define STATE_CURRENT_EMPTY   (void*) 0

void State_starter(State* state, unsigned short value) {
  state->current = STATE_CURRENT_EMPTY;
  state->from_clock = Timertask_getClock_Override();
  state->subtask = 0;
  state->timeout = value;
  state->failures = 0;
}

void State_release(State* state) {
  if (state->current != STATE_CURRENT_EMPTY) {
    state->current->fused = 0; // deactivate current statetask
    Timertask_fromNow(&state->current->task); // earliest execution is now + delay
  }
  state->current = STATE_CURRENT_EMPTY;
  state->subtask = 0;
}

char State_isReady(State* state, Statetask* statetask) {
  unsigned short delta;
  if (state->current) {
    delta = Timertask_getClock_Override() - state->from_clock;
    if (state->timeout <= delta) {
      ++state->failures;
      State_timeout_Override(state); // notification that state is about to be canceled
      State_release(state); // make sure this culprit is removed
      return 0;
    }
    return state->current == statetask;
  }
  // --- at this point a new statetask is initialized
  state->current = statetask;
  state->from_clock = Timertask_getClock_Override();
  state->subtask = 0; // this line is probably obsolete, but we keep it to play safe
  return 1;
}

// ---

void State_initStatetask(Statetask* statetask, unsigned short delay) {
  Timertask_setRepeated(&(*statetask).task, delay);
  statetask->fused = 0;
}

char State_isActive(State* state, Statetask* statetask) {
  if (Timertask_isReady(&(*statetask).task))
    statetask->fused = 1;
  if (statetask->fused) // conditions need to be separate
    return State_isReady(state, statetask);
  return 0;
}

void State_turing(State* state, Statetask* statetask, char (*state_func)(char*)) {
  if (State_isActive(state, statetask))
    if (state_func(&state->subtask))
      State_release(state);
}
