// code by ntu ael

#include <c8051F120.h>

#include "C8051_comparator.h"
#include "C8051_delay.h"

// Overview:        This function will be executed to disable Comaparator0.
void Comparator0_close() {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = CPT0_PAGE;

  // SFR Definition 10.1. CPT0CN: Comparator0 Control (Refer to page 122)
  // Bit7: CP0EN: Comparator0 Enable Bit.
  //  0: Comparator0 Disabled. // 1: Comparator0 Enabled.
  // Bit6: CP0OUT: Comparator0 Output State Flag.
  //  0: Voltage on CP0+ < CP0–. //  1: Voltage on CP0+ > CP0–.
  // Bit5: CP0RIF: Comparator0 Rising-Edge Flag.
  //  0: No Comparator0 Rising Edge has occurred since this flag was last cleared.
  //  1: Comparator0 Rising Edge has occurred.
  // Bit4: CP0FIF: Comparator0 Falling-Edge Flag.
  //  0: No Comparator0 Falling-Edge has occurred since this flag was last cleared.
  //  1: Comparator0 Falling-Edge has occurred.
  // Bits3–2: CP0HYP1–0: Comparator0 Positive Hysteresis Control Bits.
  //  00: Positive Hysteresis Disabled.
  //  01: Positive Hysteresis = 5 mV.
  //  10: Positive Hysteresis = 10 mV.
  //  11: Positive Hysteresis = 15 mV.
  // Bits1–0: CP0HYN1–0: Comparator0 Negative Hysteresis Control Bits.
  //  00: Negative Hysteresis Disabled.
  //  01: Negative Hysteresis = 5 mV.
  //  10: Negative Hysteresis = 10 mV.
  //  11: Negative Hysteresis = 15 mV.

  // Comparator 0 is disabled
  CPT0CN = 0x00; // Refer to page 122
  Delay_us(20); // Wait 20us for initialization

  // SFR Definition 10.2. CPT0MD: Comparator0 Mode Selection
  // Bits7–6: UNUSED. Read = 00b, Write = don’t care.
  // Bit 5: CP0RIE: Comparator 0 Rising-Edge Interrupt Enable Bit.
  //  0: Comparator 0 rising-edge interrupt disabled.
  //  1: Comparator 0 rising-edge interrupt enabled.
  // Bit 4: CP0FIE: Comparator 0 Falling-Edge Interrupt Enable Bit.
  //  0: Comparator 0 falling-edge interrupt disabled.
  //  1: Comparator 0 falling-edge interrupt enabled.
  // Bits3–2: UNUSED. Read = 00b, Write = don’t care.
  // Bits1–0: CP0MD1–CP0MD0: Comparator0 Mode Select
  // These bits select the response time for Comparator0

  // disabled both rising and falling edge interrupt, selected lowest power consumption
  CPT0MD = 0x03; // Refer to page 123
  EIE1 &= 0xCF; // disable CP0 rising and falling edge in interrupt enable register
  SFRPAGE = SFRPAGE_SAVE;
}

// Overview:        This function will be executed to disable Comaparator1.
void Comparator1_close() {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = CPT1_PAGE;

  // SFR Definition 10.3. CPT1CN: Comparator1 Control (Refer to page 124)
  // Bit7: CP1EN: Comparator1 Enable Bit.
  //  0: Comparator1 Disabled.
  //  1: Comparator1 Enabled.
  // Bit6: CP1OUT: Comparator1 Output State Flag.
  //  0: Voltage on CP1+ < CP1–.
  //  1: Voltage on CP1+ > CP1–.
  // Bit5: CP1RIF: Comparator1 Rising-Edge Flag.
  //  0: No Comparator1 Rising Edge has occurred since this flag was last cleared.
  //  1: Comparator1 Rising Edge has occurred.
  // Bit4: CP1FIF: Comparator1 Falling-Edge Flag.
  //  0: No Comparator1 Falling-Edge has occurred since this flag was last cleared.
  //  1: Comparator1 Falling-Edge Interrupt has occurred.
  // Bits3–2: CP1HYP1–0: Comparator1 Positive Hysteresis Control Bits.
  //  00: Positive Hysteresis Disabled.
  //  01: Positive Hysteresis = 5 mV.
  //  10: Positive Hysteresis = 10 mV.
  //  11: Positive Hysteresis = 15 mV.
  // Bits1–0: CP1HYN1–0: Comparator1 Negative Hysteresis Control Bits.
  //  00: Negative Hysteresis Disabled.
  //  01: Negative Hysteresis = 5 mV.
  //  10: Negative Hysteresis = 10 mV.
  //  11: Negative Hysteresis = 15 mV.

  // Comparator 1 is disabled
  CPT1CN = 0x00; // Refer to page 124
  Delay_us(20); // Wait 20us for initialization
  // SFR Definition 10.4. CPT1MD: Comparator1 Mode Selection
  // Bits7–6: UNUSED. Read = 00b, Write = don’t care.
  // Bit 5: CP1RIE: Comparator 1 Rising-Edge Interrupt Enable Bit.
  //  0: Comparator 1 rising-edge interrupt disabled.
  //  1: Comparator 1 rising-edge interrupt enabled.
  // Bit 4: CP1FIE: Comparator 0 Falling-Edge Interrupt Enable Bit.
  //  0: Comparator 1 falling-edge interrupt disabled.
  //  1: Comparator 1 falling-edge interrupt enabled.
  // Bits3–2: UNUSED. Read = 00b, Write = don’t care.
  // Bits1–0: CP1MD1–CP1MD0: Comparator1 Mode Select
  // These bits select the response time for Comparator1.

  // disabled both rising and falling edge interrupt, selected lowest power consumption
  CPT1MD = 0x03; // Refer to page 125
  EIE1 &= 0x3F; // disable CP1 rising and falling edge in interrupt enable register
  SFRPAGE = SFRPAGE_SAVE;
}
