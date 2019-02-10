// code by ntu ael

#include "Velox_screen.h"
#include "Velox_uptime.h"

char Seconds_haveAdvanced() {
  char value;
  Screen_capture();
  value = Uptime_handler();
  if (value) // true once per second
    Screen_summary();
  return value;
}
