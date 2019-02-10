// code by ntu ael

#include <c8051f120.h>

#include "Velox_timertask.h"
#include "Velox_watchdog.h"

volatile char clock_interruptFlag = 0;
volatile unsigned short clock_interruptMs = 0;

extern unsigned char watchdog_IE;

unsigned short Timertask_getClock_Override() {
  unsigned short value;
  IE &= ~watchdog_IE;
  value = clock_interruptMs;
  IE |= watchdog_IE;
  return value;
}

// called by the application defined 'milli-seconds counting interrupt'
// the mask clock_IE defined by Clock_setIE is used block the interrupt in order to access the value clock_interruptMs
void Clock_interruptCallback() {
  Watchdog_interruptCallback();
  ++clock_interruptMs;
  clock_interruptFlag = 1;
}

// called in main loop to notify advanced watchdog that main loop still running
char Clock_hasAdvanced() {
  char value = clock_interruptFlag; // no need to disable interrupts for boolean flags
  if (value) {
    clock_interruptFlag = 0; // no need to disable interrupts
    Watchdog_confirmAlive();
  }
  return value;
}
