// code by ntu ael

#include "Velox_macros.h"
#include "Velox_screen.h"
#include "Velox_timertask.h"

// TODO needs to be initialized somehow... low priority

void Screen_capture() {
  short toc = Timertask_getClock_Override();
  screen.timing.lastLoopDuration = toc - screen.timing.currentLoopTic;
  screen.timing.maxLoopsDuration = max(screen.timing.maxLoopsDuration, screen.timing.lastLoopDuration);
  screen.timing.currentLoopTic = toc;
  ++screen.iterations.currentLoopsCount;
}

void Screen_summary() {
  screen.timing.maxLoopsDuration = 0;
  // ---
  screen.iterations.lastLoopsCount = screen.iterations.currentLoopsCount;
  screen.iterations.currentLoopsCount = 0;
}
