// code by ntu ael

#include <c8051f120.h>

#include "C8051_mcutemp.h"
#include "C8051_SMBus.h"
#include "C8051_SMBusTimer3.h"
#include "C8051_SPI0_shared.h"
#include "C8051_UART0.h"
#include "C8051_UART1.h"

#include "Velox_macros.h"
#include "Velox_meta.h"
#include "Velox_sfrstack.h"
#include "Velox_watchdog.h"

// preconditions:
// 1) Oscillator_init() called
// 2) SFR_init() called
void Meta_init() {
  // c8051
  chip_meta.oscillator = Oscillator_getType();
  chip_meta.systemFreq = Oscillator_getSystemClock();
  SFR_pushPage(LEGACY_PAGE);
  chip_meta.resetSource = RSTSRC;
  SFR_popPage();
  // according to page 182
  // Note: When Bit 1 (from 7...0) of RSTSRC is read as '1', all other reset flags are indeterminate.
  if (chip_meta.resetSource & 2) // Power-On Reset Flag
    chip_meta.resetSource = 2;
  // velox
  chip_meta.totalResets = 0; // in the init process, the field is updated using permanent storage
  chip_meta.watchdogCount = 0; // important init to trace max
}

void Meta_handler() {
  // c8051
  chip_meta.spi0_failures = SPI0_getFailureCount();
  chip_meta.mcuTemperature = Mcutemp_inCelsius();
  // velox
  SMBus_getStatusCount(chip_meta.smbus_statusCount);
  chip_meta.smbus_timeoutCount = SMBusTimer3_getTimeoutCount();
  chip_meta.uart0_failureCount = UART0_getRxErrorCount() + UART0_getTxErrorCount();
  chip_meta.uart1_failureCount = UART1_getTxErrorCount(); // uart1 does not feature rx error count
  chip_meta.watchdogCount = max(chip_meta.watchdogCount, Watchdog_getMaxCount());
  chip_meta.sfr_depthStatus = SFR_getMaxDepth();
}
