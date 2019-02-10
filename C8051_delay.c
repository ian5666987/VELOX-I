// code by ntu ael

#include <c8051F120.h>

#include "C8051_oscillator.h"

#define DELAY_SYSCLK_FACTOR    2.2605613425e-8   // 1 / 22118400.0 = 4.52112268e-8 ->  / 2  -> 2.2605613425e-8
// Delay functions design to delay by 1us. Call Format : delay_us(Number)
// function: delay for (l_us_us_count) micro-seconds
// note: l_us_us_count = l_us_us_count/2;
// this estimation is gotten by testing the delay using
// an oscilloscope for many inputs of l_us_us_count, and provides
// a rough estimate of doing micro-seconds delay
// Actual delay is 57% of the specified delay. (20s finished in 11.4s)
// with 24.5MHz system clock, it is expected to be 90%. (20s finished in 18s)
// with 22.1184MHz system clock, it is expected to be 99.75% (20s finished in 19.95s) 
// So, the delay is now adjusted using some calculation 
void Delay_us(unsigned short l_us_us_count) {
  float factor = Oscillator_getSystemClock() * DELAY_SYSCLK_FACTOR;
  factor *= l_us_us_count;
  l_us_us_count = 65534 < factor ? 65535 : factor;
  while (l_us_us_count)
    --l_us_us_count;
}

// for delay in milli seconds use the function Delay_blockMs in Velox_delay
