// code by ntu ael

#define  interrupt(x)     interrupt x

#include <c8051F120.h>

#include "C8051_delay.h"
#include "C8051_SPI0_shared.h"

volatile char* spi0_txMessage;
volatile char* spi0_rxMessage;

volatile short spi0_txPosition;
volatile short spi0_rxPosition;
volatile short spi0_available; // disable interrupts when reading value in main code
volatile char spi0_rxIncrement;

extern short SPI0_failures;

// EA is required to be on prior to calling this function
char SPI0int_process(char mode, void* buffer, short length) {
  char SFRPAGE_SAVE;
  char dummy;
  if (0 < length) { // negative or zero length => nothing is sent; length has to be strictly positive
    spi0_txMessage = (char*) buffer;
    spi0_txPosition = 0; // position of next write
    spi0_rxMessage = mode ? (char*) buffer : &dummy;
    spi0_rxPosition = 0; // position of next read
    spi0_rxIncrement = mode;
    SFRPAGE_SAVE = SFRPAGE;
    SFRPAGE = SPI0_PAGE;
    spi0_available = length; // strictly positive
    EIE1 |= ESPI0_MASK; // enable spi interrupt status
    SPI0DAT = spi0_txMessage[spi0_txPosition]; // send first in main code
    while (1) { // FIXME need a timeout criteria based on _facts_ ! note: any exit criteria needs to disable spi0 interrupt
      Delay_us(5);
      EIE1 &= ~ESPI0_MASK; // Disable spi0 interrupts
      if (spi0_available == 0)
        break; // wait until interrupt has rx length bytes
      EIE1 |= ESPI0_MASK; // re-enable spi interrupt status
    }
    SFRPAGE = SFRPAGE_SAVE;
  }
  return 1;
}

SPI0_ISR() interrupt (6) { // page 155
  if (SPIF) { // tx complete
    SPIF = 0;
    spi0_rxMessage[spi0_rxPosition] = SPI0DAT; // recv
    spi0_rxPosition += spi0_rxIncrement;
    --spi0_available;
    if (spi0_available) {
      ++spi0_txPosition; // move pointer to next byte
      SPI0DAT = spi0_txMessage[spi0_txPosition]; // write
    }
  }
  if (SPI0CN & 0x70) {
    ++SPI0_failures;
    SPI0CN &= 0x8f; // clear remaining flags
  }
}
