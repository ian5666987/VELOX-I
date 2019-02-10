// code by ntu ael

#include <c8051F120.h>

#include "C8051_SPI0.h"
#include "C8051_delay.h"

extern short SPI0_failures;
extern short SPI0_timeout; // default timeout

// private function, use SPI0_getPutChars instead ! warning this page does not need to set SFRPAGE = SPI0_PAGE;
// Min-max time: 0.005-0.05ms with SPI0_timeout = 25
char SPI0_getPut(char* value) {
  short delay;
  // ---
  delay = 0;
  while (SPI0CFG & 0x80) // busy
    if (SPI0_timeout < ++delay) {
      ++SPI0_failures;
      return 0;
      break;
    }
  // ---
  SPI0DAT = *value; // send
  // ---
  delay = 0;
  while (!SPIF)
    if (SPI0_timeout < ++delay) {
      ++SPI0_failures;
      break;
    }
  // ---
  *value = SPI0DAT; // recv
  SPIF = 0;
  return 1;
}

// function writes length chars from ptr to SPI and simultaneously writes reply into ptr
void SPI0_putGetChars(void* buffer, short length) {
  short count;
  char* message = (char*) buffer;
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = SPI0_PAGE;
  for (count = 0; count < length; ++count)
    SPI0_getPut(&message[count]); // return value is ignored
  SFRPAGE = SFRPAGE_SAVE;
}

// function writes length chars from ptr to SPI
// the reply is dropped on purpose
void SPI0_putChars(void* buffer, short length) {
  short count;
  char* message = (char*) buffer;
  char temp;
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = SPI0_PAGE;
  for (count = 0; count < length; ++count) {
    temp = message[count];
    SPI0_getPut(&temp); // reply is discarded, return value is ignored
  }
  SFRPAGE = SFRPAGE_SAVE;
}
