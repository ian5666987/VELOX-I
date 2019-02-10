// code by ntu ael

#define  interrupt(x)     interrupt x

#include <c8051F120.h>

#include "C8051_extern_int.h"

// This function configures and enables /INT0 and /INT1 (External Interrupts)
// as negative edge-triggered.
// Overview:        This function will be executed to configure and initialize External Interrupt.
void ExternalInterrupt_Disable() {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = TIMER01_PAGE;

  // SFR Definition 23.1. TCON: Timer Control
  // Bit7: TF1: Timer 1 Overflow Flag.
  // Set by hardware when Timer 1 overflows. This flag can be cleared by software but is automatically cleared when the
  // CPU vectors to the Timer 1 interrupt service routine.
  //  0: No Timer 1 overflow detected.
  //  1: Timer 1 has overflowed.

  // Bit6: TR1: Timer 1 Run Control.
  //  0: Timer 1 disabled.
  //  1: Timer 1 enabled.

  // Bit5: TF0: Timer 0 Overflow Flag.
  // Set by hardware when Timer 0 overflows. This flag can be cleared by software but is automatically cleared when the
  // CPU vectors to the Timer 0 interrupt service routine.
  //  0: No Timer 0 overflow detected.
  //  1: Timer 0 has overflowed.

  // Bit4: TR0: Timer 0 Run Control.
  //  0: Timer 0 disabled.
  //  1: Timer 0 enabled.

  // Bit3: IE1: External Interrupt 1.
  // This flag is set by hardware when an edge/level of type defined by IT1 is detected. It can be cleared by software
  // but is automatically cleared when the CPU vectors to the External Interrupt 1 service routine if IT1 = 1. This flag
  // is the inverse of the /INT1 signal.

  // Bit2: IT1: Interrupt 1 Type Select.
  // This bit selects whether the configured /INT1 interrupt will be falling-edge sensitive or active-low.
  //  0: /INT1 is level triggered, active-low.
  //  1: /INT1 is edge triggered, falling-edge.

  // Bit1: IE0: External Interrupt 0.
  // This flag is set by hardware when an edge/level of type defined by IT0 is detected. It can be cleared by software
  // but is automatically cleared when the CPU vectors to the External Interrupt 0 service routine if IT0 = 1. This flag
  // is the inverse of the /INT0 signal.

  // Bit0: IT0: Interrupt 0 Type Select.
  // This bit selects whether the configured /INT0 interrupt will be falling-edge sensitive or active-low.
  //  0: /INT0 is level triggered, active logic-low.
  //  1: /INT0 is edge triggered, falling-edge.

  TCON |= 0x05; // Set IT1 & IT0 to edge triggered, falling edged.

  EX0 = 0; // disable INT0_N interrupt
  EX1 = 0; // disable INT1_N interrupt

  SFRPAGE = SFRPAGE_SAVE;
}

EXT_INT0() interrupt (0) {
}

EXT_INT1() interrupt (2) {
}
