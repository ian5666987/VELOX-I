// code by ntu ael

#include <c8051F120.h>

sfr16 RCAP4 = 0xCA; // Timer 4 reload register
sfr16 TMR4 = 0xCC; // Timer 4 register

// Input: sys_clk : system clock
// baudrate : desired baudrate to be set
// Function: Used for UART0 baudrate generator
void UART0Timer4_init(long sys_clk, long baudrate) {
  char SFRPAGE_SAVE = SFRPAGE; // Save current SFR page
  short timer4_reload;

  //Switch to Timer 4
  SFRPAGE = TMR4_PAGE;

  //Timer 4 in 16-bit auto-reload up timer mode
  TMR4CN = 0x00;

  //SYSCLK is time base; no output, up count only
  TMR4CF = 0x08;
  timer4_reload = (float) sys_clk / baudrate / 16 + 0.5;
  RCAP4 = -(timer4_reload);

  TMR4 = RCAP4;

  EIE2 &= 0xFB; //to disble T4 int, added by SWL 19 Mar 12
  //Start Timer 4
  TR4 = 1;

  //Restore SFRPAGE
  SFRPAGE = SFRPAGE_SAVE;
}

