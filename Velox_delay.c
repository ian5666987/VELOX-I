// code by ntu ael

#include <intrins.h>

#include "C8051_delay.h"

#include "Velox_delay.h"
#include "Velox_timertask.h"

// delay is at least 'value' ms.
// in fact, delay is between 'value' ms and 'value+1' ms
// Delay_blockMs requires EA==1 to perform!
void Delay_blockMs(short value) {
  struct struct_timertask delay_task;
  Timertask_setExecuteOnce(&delay_task, value + 1); // +1 guarantees that the delay is at least 'value' ms
  while (!Timertask_isReady(&delay_task)) {
    Delay_us(10);
    _nop_();
  }
}
