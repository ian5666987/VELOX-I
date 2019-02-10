// code by ntu ael

#include <c8051F120.h>

#include "C8051_watchdog.h"

// This routine initializes the Watchdog Timer for a maximum period of 10.49 ms
// with WDTCN[2-0] = 111b and SYSCLK = 100 MHz:
// 4 ^ (3 + WDTCN[2-0]) x SYSCLK = 4 ^ (3 + 7) x 1/(100 MHz) = 10.49 ms

// Overview:        This function initialises the watchdog timer (WDT)
void Watchdog_init() {
  //WDTCN: Watchdog Timer Control
  //Bits7-0: WDT Control
  //Writing 0xA5 both enables and reloads the WDT.
  //Writing 0xDE followed within 4 system clocks by 0xAD disables the WDT.
  //Writing 0xFF locks out the disable feature.
  //Bit4: Watchdog Status Bit (when Read)
  //Reading the WDTCN.[4] bit indicates the Watchdog Timer Status.
  //0: WDT is inactive
  //1: WDT is active
  //Bits2-0: Watchdog Timeout Interval Bits
  //The WDTCN.[2:0] bits set the Watchdog Timeout Interval
  //When writing these bits, WDTCN.7 must be set to 0.

  //WDTCN.7 must be logic 0 when setting the interval
  WDTCN &= 0x7F;
  //Set the WDTCN[2-0] to 111b
  WDTCN |= 0x07;
  //Reset the WDT
  WDTCN = 0xA5;
}

// Overview:        This function resets the WDT
void Watchdog_resetTimer() {
  WDTCN = 0xA5; // no need to change SFRPAGE (see page 144)
}

// Overview:        This function will disables the watchdog timer
void Watchdog_disable() {
//  The writes of 0xDE and 0xAD must occur within 4 clock cycles of each other, or the disable operation is
//  ignored. This means that the prefetch engine should be enabled and interrupts should be disabled during
//  this procedure to avoid any delay between the two writes.
  // no need to change SFRPAGE (see page 144)
  bit EA_SAVE = EA;
  EA = 0;
  EA = 0;
  WDTCN = 0xDE;
  WDTCN = 0xAD;
  EA = EA_SAVE;
}

// Overview:        This function will locks out the disable feature of the WDT.
//                  Once locked out, the disable operation is ignored until the next
// system reset. Writing 0xFF does not enable or reset the watchdog timer. Applications always
// intending to use the watchdog should write 0xFF to WDTCN in the initialization code.
void Watchdog_lockout() {
  WDTCN = 0xFF; // no criteria will prevent this from happening, simple as that.
}
