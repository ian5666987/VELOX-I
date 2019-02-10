// code by ntu ael

#include <c8051F120.h>

#include "Velox_sfrstack.h"

char* SFR_stack;
char SFR_stackSize = 0;
char SFR_index = 0;
// ---
unsigned char SFR_maxDepth = 0;

void SFR_init(char* stack, short stack_size) {
  SFR_stack = stack;
  SFR_stackSize = stack_size;
}

void SFR_pushPage(char page) {
  SFR_stack[SFR_index] = SFRPAGE;
  if (SFR_index < SFR_stackSize - 1)
    ++SFR_index;
  else
    SFR_maxDepth |= SFRSTACK_OVERRUN;
  if (SFR_maxDepth < SFR_index)
    SFR_maxDepth = SFR_index;
  SFRPAGE = page;
}

void SFR_popPage() {
  if (SFR_index)
    --SFR_index;
  else
    SFR_maxDepth |= SFRSTACK_UNDERRUN;
  SFRPAGE = SFR_stack[SFR_index];
}

char SFR_getMaxDepth() {
  return SFR_maxDepth;
}
