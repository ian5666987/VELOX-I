// code by ntu ael

#include <c8051F120.h>

#include "c8051_watchdog.h"

// private variables
volatile unsigned char watchdog_IE = 0;
volatile short watchdog_count = 0; // increases by one at a rate of about 1 ms
volatile short watchdog_limit = 10000;
// ---
// watchdog_max is just for monitoring purposes and not part of the watchdog mechanism
volatile short watchdog_max = 0;

// confirmAlive should be invoked from the main loop
void Watchdog_confirmAlive() {
  IE &= ~watchdog_IE;
  if (watchdog_max < watchdog_count)
    watchdog_max = watchdog_count;
  watchdog_count = 0;
  IE |= watchdog_IE;
}

// change in behavior
void Watchdog_advancedInit(unsigned char mask, short limit) {
  watchdog_IE = mask;
  watchdog_limit = limit;
  Watchdog_confirmAlive();
  Watchdog_init(); // Initializes WDT
}

// interruptCallback should be called in the interrupt (about once every ms)
void Watchdog_interruptCallback() {
  if (watchdog_count <= watchdog_limit) {
    Watchdog_resetTimer();
    ++watchdog_count;
  }
}

short Watchdog_getMaxCount() {
  short value;
  IE &= ~watchdog_IE;
  value = watchdog_max;
  IE |= watchdog_IE;
  return value;
}
