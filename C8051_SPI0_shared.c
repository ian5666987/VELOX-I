// code by ntu ael

#include <c8051F120.h>

#include "C8051_SPI0_shared.h"

short SPI0_failures = 0;
short SPI0_timeout = 500;

// Configures SPI0 to use 4-wire Single-Master mode. The SPI timing is
// configured for Mode 0,0 (data centered on first edge of clock phase and
// SCK line low in idle state). The SPI clock is set to 1.75 MHz. The NSS pin
// is set to 1.
// adcs requires spi0cfg = 0x70 because of imu
// obdh used spi0cfg = 0x40 in the past
void SPI0_init(long sys_clk, long f_sck_max, unsigned char spi0cfg, short p_timeout) {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = SPI0_PAGE;
  // SPI0CFG = 0x70;
  // SPI0CN = 0x0D; // 4-Wire Master Mode, NSS pin 1, SPIEN
  // The equation for SPI0CKR is (SYSCLK/(2*F_SCK_MAX))-1, but this yields
  // a SPI frequency that is slightly more than 2 MHz. But, 2 MHz is the max
  // frequency spec of the EEPROM used here. So, the "-1" term is omitted
  // in the following usage:
  SPI0CKR = sys_clk / (2 * f_sck_max);
  SPI0CFG |= spi0cfg; // typically sets SPI as master
  NSSMD1 = 1; // 4-Wire Master Mode
  NSSMD0 = 1;
  SPIEN = 1; // general enable of SPI

  // library works without interrupt!
  // do not enable interrupt for SPI!

  SFRPAGE = SFRPAGE_SAVE;
  SPI0_timeout = p_timeout;
}

short SPI0_getFailureCount() {
  short copy;
  char ESPI0_save;
  ESPI0_save = EIE1 & ESPI0_MASK;
  EIE1 &= ~ESPI0_MASK; // disable spi0 interrupts
  copy = SPI0_failures;
  EIE1 |= ESPI0_save; // restore previous spi interrupt status
  return copy;
}
