// code by ntu ael

#define interrupt(x)     interrupt x

#include <c8051F120.h>

sfr16 RCAP3 = 0xCA; // Timer 3 capture/reload
sfr16 TMR3 = 0xCC; // Timer 3

#define EIE2_ET3    0x01

volatile unsigned short SMBusTimer3_count = 0;

unsigned short SMBusTimer3_getTimeoutCount() {
  unsigned short count;
  char EIE2_SAVE = EIE2 & EIE2_ET3;
  EIE2 &= 0xFE; // 1111 1110
  count = SMBusTimer3_count;
  EIE2 |= EIE2_SAVE;
  return count;
}

void SMBusTimer3_init(long sys_clk) {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = TMR3_PAGE;
  TMR3CN = 0x00; // Timer3 external enable off; Timer3 in timer mode; Timer3 in auto-reload mode
  if (sys_clk > 25000000L) { //for high gear
    TMR3CF = 0x00; // Timer3 uses sysclk/12; Timer3 output not available; Timer3 counts up
    sys_clk /= 12; // = 12*40 (later)
  } else
    TMR3CF = 0x08; // Timer3 uses sysclk; Timer3 output not available; Timer3 counts up
  sys_clk /= 40;
  if (0xFFFF < sys_clk)
    RCAP3 = 0x0000;
  else
    RCAP3 = -sys_clk; // Timer 3 configured to overflow after
  TMR3 = RCAP3; // ~25ms (for SMBus low timeout detect) (1/0.025 = 40)
  EIE2 |= EIE2_ET3; // Timer3 interrupt enable
  TR3 = 1; // Start Timer3
  SFRPAGE = SFRPAGE_SAVE; // Restore SFR page detector
}

// A Timer3 interrupt indicates an SMBus SCL low timeout.
SMBusTimer3_ISR() interrupt (14) {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = SMB0_PAGE;
  SMB0CN &= ~0x40; // Disable SMBus
  SMB0CN |= 0x40; // Re-enable SMBus
  SFRPAGE = SFRPAGE_SAVE; // Switch back to the Timer3 SFRPAGE
  ++SMBusTimer3_count;
  TF3 = 0; // Clear Timer3 interrupt-pending flag
}
