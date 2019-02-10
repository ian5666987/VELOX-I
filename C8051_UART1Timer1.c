// code by ntu ael

#include <c8051F120.h>

// Input: sys_clk : system clock
//        prescaler : timer 1 prescaler based on oscillator setting
//        baudrate : desired baudrate to be set
// This function configures the Timer1 as a 16-bit timer, interrupt disabled, T1 pin not used
// and timer itself is disabled

// VELOX-I settings:
//--------------------------
// PWRS: timer0 uses sysclk all the time. high gear timer1 uses sysclk/48, medium gear sysclk/12, gear1-2 sysclk/4, gear0 sysclk
// OBDH: high gear uses sysclk/12, else uses sysclk for both timer1 and timer0
// ADCS: high gear uses sysclk/48, else uses sysclk/12 for both timer1 and timer0
void UART1Timer1_init(long sys_clk, char prescaler, long baudrate) {
  char SFRPAGE_SAVE = SFRPAGE; // Save Current SFR page
  char timer1_reload_hi;

  // Set SFR page
  SFRPAGE = TIMER01_PAGE; //Timer 0 or 1 page

  timer1_reload_hi = (float) sys_clk / prescaler / baudrate / 2 + 0.5;
  TH1 = -(timer1_reload_hi); //Setting reload value

  //Timer1 in 8-bit mode
  //Timer0 settings are retained
  TMOD &= 0x0F;
  TMOD |= 0x20;

  //Timer1 interrupt disabled
  ET1 = 0;
  //IP |= 0x08; // set timer 1 overflow high priority

  //Timer1 ON
  TR1 = 1;
  // Restore SFR page
  SFRPAGE = SFRPAGE_SAVE;
}
