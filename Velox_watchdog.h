// code by ntu ael

#ifndef VELOX_WATCHDOG_H
#define VELOX_WATCHDOG_H

// allows application code direct access to functions in c8051
void Watchdog_disable();
void Watchdog_lockout();

void Watchdog_advancedInit(unsigned char mask, short limit);
void Watchdog_confirmAlive();
void Watchdog_interruptCallback();
short Watchdog_getMaxCount();


#endif
