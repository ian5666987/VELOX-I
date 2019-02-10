// code by ntu ael

#ifndef VELOX_SCREEN_H
#define VELOX_SCREEN_H

struct struct_timing {
  short lastLoopDuration;
  short maxLoopsDuration;
  short currentLoopTic;
};

struct struct_iterations {
  short lastLoopsCount;
  short currentLoopsCount;
};

struct struct_screen {
  struct struct_timing timing;
  struct struct_iterations iterations;
};

extern struct struct_screen screen;

void Screen_capture();
void Screen_summary();

#endif
