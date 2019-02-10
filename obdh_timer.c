// code by ntu ael

#define interrupt(x) interrupt x

#include <c8051F120.h>

#include "Velox_clock.h"

#include "obdh_systemconfig.h"
#include "obdh_timer.h"

sfr16 RCAP2 = 0xCA; //Timer 2 reload register
sfr16 TMR2 = 0xCC; //Timer 2 register

// Timer 0 Initialization
// This function configures the Timer0 as a 16-bit timer, interrupt enabled.
// Using the Ext osc. at 28636360Hz or INT osc at 24.5MHz (for prescaling, refer to
// definitions at top of script)
// Timer0 will overflow every 1ms, triggering ISR
// Note: The Timer0 uses a 1:12 prescaler for high gear and 1:1 for low gear.
#define DESIRED_NO_OF_INT_PER_SECOND 100000L
void Timer0_init(long sys_clk, char p_c_CKCON) {
  char SFRPAGE_SAVE = SFRPAGE;
  unsigned char timer0_reload;

  SFRPAGE = TIMER01_PAGE;

  timer0_reload = (sys_clk > 30000000L) ? sys_clk / 12 / DESIRED_NO_OF_INT_PER_SECOND + 0.5 : sys_clk / DESIRED_NO_OF_INT_PER_SECOND + 0.5;

  TH0 = -(timer0_reload); // ian 20130731: Apparently, OBDH does not use timer0! The values are to generate interrupts every 10us
  TL0 = -(timer0_reload);

  //TMOD: Timer Mode Register
  //Bit 7: GATE1: Timer 1 Gate Control.
  //0: Timer 1 enabled when TR1 = 1 irrespective of INT1_N logic level.
  //1: Timer 1 enabled only when TR1 = 1 AND INT1_N = logic 1.

  //Bit 6: C/T1: Counter/Timer 1 Select.
  //0: Timer Function: Timer 1 incremented by clock defined by T1M bit (CKCON.4).
  //1: Counter Function: Timer 1 incremented by high-to-low transitions on external input pin (T1).

  //Bits 5–4: T1M1–T1M0: Timer 1 Mode Select.
  //These bits select the Timer 1 operation mode.
  //T1M1  T1M0  Mode
  //0   0   Mode 0: 13-bit counter/timer
  //0   1   Mode 1: 16-bit counter/timer
  //1   0   Mode 2: 8-bit counter/timer with auto-reload
  //1   1   Mode 3: Timer 1 inactive

  //Bit 3: GATE0: Timer 0 Gate Control.
  //0: Timer 0 enabled when TR0 = 1 irrespective of /INT0 logic level.
  //1: Timer 0 enabled only when TR0 = 1 AND /INT0 = logic 1.

  //Bit 2: C/T0: Counter/Timer Select.
  //0: Timer Function: Timer 0 incremented by clock defined by T0M bit (CKCON.3).
  //1: Counter Function: Timer 0 incremented by high-to-low transitions on external input pin (T0).

  //Bits 1–0: T0M1–T0M0: Timer 0 Mode Select.
  //These bits select the Timer 0 operation mode.
  //T0M1  T0M0  Mode
  //0   0   Mode 0: 13-bit counter/timer
  //0   1   Mode 1: 16-bit counter/timer
  //1   0   Mode 2: 8-bit counter/timer with auto-reload
  //1   1   Mode 3: Two 8-bit counter/timers

  //Timer0 in 8-bit counter/timer with auto-reload
  //Timer1 settings are retained
  TMOD &= 0xF0;
  TMOD |= 0x02;

  //CKCON : Clock Control
  //Bits 7–5: UNUSED. Read = 000b, Write = don’t care.
  //Bit 4: T1M: Timer 1 Clock Select.
  //This select the clock source supplied to Timer 1. T1M is ignored when C/T1 is set to logic 1.
  //0: Timer 1 uses the clock defined by the prescale bits, SCA1–SCA0.
  //1: Timer 1 uses the system clock.
  //Bit 3: T0M: Timer 0 Clock Select.
  //This bit selects the clock source supplied to Timer 0. T0M is ignored when C/T0 is set to logic 1.
  //0: Counter/Timer 0 uses the clock defined by the prescale bits, SCA1-SCA0.
  //1: Counter/Timer 0 uses the system clock.
  //Bit 2: UNUSED. Read = 0b, Write = don’t care.
  //Bits 1–0: SCA1–SCA0: Timer 0/1 Prescale Bits
  //These bits control the division of the clock supplied to Timer 0 and/or Timer 1 if configured to use prescaled clock inputs.
  //SCA1  SCA0  Prescaled Clock
  //0   0   System clock divided by 12
  //0   1   System clock divided by 4
  //1   0   System clock divided by 48
  //1   1   External clock divided by 8

  CKCON = p_c_CKCON;
  ET0 = 0; // Interrupt disabled
  TR0 = 1; // Timer0 ON
  SFRPAGE = SFRPAGE_SAVE;
}

// Timer2_Init
// function: auto reload mode with no interrupt, not running
// ian 20130215 - Timer2 interrupt is now used to produce periodical interrupt
// the periodical interrupt is used for timing, the timing is used for scheduler
#define DESIRED_TIMER2_INT_PERIOD_MS 1 //must be changed
#define TIMER2_PRESCALER 12 //choice: 1, 2, 12, 8 + external clock
void Timer2_init(long sys_clk) {
  char SFRPAGE_SAVE = SFRPAGE; // Save Current SFR page
  SFRPAGE = TMR2_PAGE;
  TMR2CN = 0x00; // Timer2 external enable off;
                 // Timer2 in timer mode;
  // Transactions on T2EX ignored
  // Timer2 in auto-reload mode
  // Timer2 output not available on pin T2
  // Timer2 counts up
  TMR2CF = 0x00;

#if (TIMER2_PRESCALER == 12)
#elif (TIMER2_PRESCALER == 1)
  TMR2CF |= 0x08; // Timer2 uses sysclk
#elif (TIMER2_PRESCALER == 2)
  TMR2CF |= 0x18; // Timer2 uses sysclk/2
#elif (TIMER2_PRESCALER == 8) //Assume external clock is used
  TMR2CF |= 0x10;// external clock/8
#else //Assume pre-scaler 12 is used
#endif

  RCAP2 = -(sys_clk / TIMER2_PRESCALER / 1000 * DESIRED_TIMER2_INT_PERIOD_MS); // Timer2 value is set that its interrupt could be called every 1ms
  TMR2 = RCAP2; // ~25ms (for SMBus low timeout detect)
  ET2 = 1; // timer 2 interrupt is enabled
  TR2 = 1; // start Timer2
  SFRPAGE = SFRPAGE_SAVE; // Restore SFR page detector
}

Timer2_ISR() interrupt (5) { // page 155
  TF2 = 0; // Clear Timer2 interrupt-pending flag
  Clock_interruptCallback();
}
