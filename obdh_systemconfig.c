// code by ntu ael

#include "mccif_buffer.h"

#include "obdh_constant.h"
#include "obdh_systemconfig.h"

char System_getT1Prescaler(OscGear osc_gear) {
	return osc_gear >= oscillator_gear4 ? 12 : 1; //for high gear uses sysclk/12, else uses sysclk
}

char System_getCKCON(OscGear osc_gear) {
	return osc_gear >= oscillator_gear4 ? 0x00 : 0x18; //for high gear uses sysclk/12, else uses sysclk for both timer1 and timer0
}

// routine overloads the standard putchar() library function to support
char putchar(char value) {
  Mccif_appendChar(PRINTF_INDEX, value);
  return value;
}
